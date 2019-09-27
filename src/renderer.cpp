
#include "adm_helper.hpp"
#include "renderer.hpp"
#include "parser.hpp"

namespace admrenderer {

Renderer::Renderer(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
           const std::string& outputLayout,
           const std::string& outputDirectory,
           const float dialogGain)
  : _inputFile(inputFile)
  , _inputNbChannels(inputFile->channels())
  , _outputLayout(ear::getLayout(outputLayout))
  , _outputDirectory(outputDirectory)
  , _dialogGain(dialogGain)
{
}

void Renderer::process() {
  auto admDocument = getAdmDocument(parseAdmXmlChunk(_inputFile));

  /// Based on Rec. ITU-R  BS.2127-0, 5.2 Determination of Rendering Items (Fig. 3)
  auto audioProgrammes = admDocument->getElements<adm::AudioProgramme>();
  if(audioProgrammes.size()) {
    for(auto audioProgramme : audioProgrammes) {
      processAudioProgramme(audioProgramme);
    }
    return;
  }

  auto audioObjects = admDocument->getElements<adm::AudioObject>();
  if(audioObjects.size()) {
    for(auto audioObject : audioObjects) {
      processAudioObject(audioObject);
    }
    return;
  }

  auto chnaChunk = parseAdmChnaChunk(_inputFile);
  if (chnaChunk) {
    std::vector<bw64::AudioId> audioIds = chnaChunk->audioIds();
    if(audioIds.size()) {
      // TODO: parse to get AudioTrackUID, AudioPackFormat and AudioTrackFormat, and render
      return;
    }
  }
}

void Renderer::processAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
  std::cout << "### Render audio programme: " << toString(audioProgramme) << std::endl;

  for(const std::shared_ptr<adm::AudioObject> audioObject : getAudioObjects(audioProgramme)) {
    AudioObjectRenderer renderer(_outputLayout, audioObject);
    std::cout << " >> Add renderer: " << renderer << std::endl;
    _renderers.push_back(renderer);
  }

  // Create output programme ADM
  std::shared_ptr<adm::Document> document = createAdmDocument(audioProgramme, _outputLayout);
  std::shared_ptr<bw64::AxmlChunk> axml = createAxmlChunk(document);
  std::shared_ptr<bw64::ChnaChunk> chna = createChnaChunk(document);

  // Output file
  std::stringstream outputFileName;
  outputFileName << _outputDirectory;
  if(_outputDirectory.back() != std::string(PATH_SEPARATOR).back()) {
    outputFileName << PATH_SEPARATOR;
  }
  outputFileName << audioProgramme->get<adm::AudioProgrammeName>().get() << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), _outputLayout.channels().size(), _inputFile->sampleRate(), _inputFile->bitDepth(), chna, axml);

  toFile(outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

void Renderer::processAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject) {
  std::cout << "### Render audio object: " << toString(audioObject) << std::endl;

  AudioObjectRenderer renderer(_outputLayout, audioObject);
  std::cout << " >> Add renderer: " << renderer << std::endl;
  _renderers.push_back(renderer);

  // Create output programme ADM
  std::shared_ptr<adm::Document> document = createAdmDocument(audioObject, _outputLayout);
  std::shared_ptr<bw64::AxmlChunk> axml = createAxmlChunk(document);
  std::shared_ptr<bw64::ChnaChunk> chna = createChnaChunk(document);

  // Output file
  std::stringstream outputFileName;
  outputFileName << _outputDirectory;
  if(_outputDirectory.back() != std::string(PATH_SEPARATOR).back()) {
    outputFileName << PATH_SEPARATOR;
  }
  outputFileName << audioObject->get<adm::AudioObjectName>().get() << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), _outputLayout.channels().size(), _inputFile->sampleRate(), _inputFile->bitDepth(), chna, axml);

  toFile(outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

size_t Renderer::processBlock(const size_t nbFrames, const float* input, float* output) {
  const size_t outputNbChannels = _outputLayout.channels().size();
  size_t frame = 0;
  size_t read = 0;
  size_t written = 0;

  while(frame < nbFrames) {
    float* ocframe = &output[written];
    const float* icframe = &input[read];
    for(AudioObjectRenderer renderer : _renderers) {
      renderer.renderAudioFrame(icframe, ocframe);
    }
    written += _outputLayout.channels().size();
    read += _inputNbChannels;
    frame++;
  }
  return written;
}

void Renderer::toFile(const std::unique_ptr<bw64::Bw64Writer>& outputFile) {

  // Buffers
  const size_t outputNbChannels = outputFile->channels();

  // Read file, render with gains and write output file
  float inputBuffer[BLOCK_SIZE * _inputNbChannels] = {0.0,}; // nb of samples * nb input channels
  while (!_inputFile->eof()) {
    // Read a data block
    float outputBuffer[BLOCK_SIZE * outputNbChannels] = {0.0,}; // nb of samples * nb output channels
    auto nbFrames = _inputFile->read(inputBuffer, BLOCK_SIZE);
    processBlock(nbFrames, inputBuffer, outputBuffer);
    outputFile->write(outputBuffer, nbFrames);
  }
  _inputFile->seek(0);
}

}
