
#include "adm_helper.hpp"
#include "renderer.hpp"
#include "parser.hpp"
#include "utils.hpp"

namespace admengine {

Renderer::Renderer(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
           const std::string& outputLayout,
           const std::string& outputDirectory,
           const std::map<std::string, float> elementGains,
           const std::string& elementIdToRender)
  : _inputFile(inputFile)
  , _inputNbChannels(inputFile->channels())
  , _outputLayout(ear::getLayout(outputLayout))
  , _outputDirectory(outputDirectory)
  , _elementGainsMap(elementGains)
  , _elementIdToRender(elementIdToRender)
{
  _admDocument = getAdmDocument(parseAdmXmlChunk(_inputFile));
  _chnaChunk = parseAdmChnaChunk(_inputFile);
}

void Renderer::process() {
  // if the user selected an item ID to render, find it and render
  if(!_elementIdToRender.empty()) {
    for(auto audioProgramme : getDocumentAudioProgrammes()) {
      if(_elementIdToRender == formatId(audioProgramme->get<adm::AudioProgrammeId>())) {
        std::cout << "### Render audio programme: " << toString(audioProgramme) << std::endl;
        initAudioProgrammeRendering(audioProgramme);
        processAudioProgramme(audioProgramme);
        return;
      }
    }
    for(auto audioObject : getDocumentAudioObjects()) {
      if(_elementIdToRender == formatId(audioObject->get<adm::AudioObjectId>())) {
        std::cout << "### Render audio object: " << toString(audioObject) << std::endl;
        initAudioObjectRendering(audioObject);
        processAudioObject(audioObject);
        return;
      }
    }
    std::stringstream message;
    message << "Could not find any audio element from ID: '" << _elementIdToRender << "'. ";
    throw std::runtime_error(message.str());
  }

  // otherwise select items to render, based on Rec. ITU-R  BS.2127-0, 5.2 Determination of Rendering Items (Fig. 3)
  auto audioProgrammes = getDocumentAudioProgrammes();
  if(audioProgrammes.size()) {
    for(auto audioProgramme : audioProgrammes) {
      std::cout << "### Render audio programme: " << toString(audioProgramme) << std::endl;
      initAudioProgrammeRendering(audioProgramme);
      processAudioProgramme(audioProgramme);
    }
    return;
  }

  auto audioObjects = getDocumentAudioObjects();
  if(audioObjects.size()) {
    for(auto audioObject : audioObjects) {
      std::cout << "### Render audio object: " << toString(audioObject) << std::endl;
      initAudioObjectRendering(audioObject);
      processAudioObject(audioObject);
    }
    return;
  }

  if (_chnaChunk) {
    std::vector<bw64::AudioId> audioIds = _chnaChunk->audioIds();
    if(audioIds.size()) {
      // TODO: parse to get AudioTrackUID, AudioPackFormat and AudioTrackFormat, and render
      return;
    }
  }
}

std::vector<std::shared_ptr<adm::AudioProgramme>> Renderer::getDocumentAudioProgrammes() {
  std::vector<std::shared_ptr<adm::AudioProgramme>> programmes;
  for(auto programme : _admDocument->getElements<adm::AudioProgramme>()) {
    programmes.push_back(programme);
  }
  return programmes;
}

std::vector<std::shared_ptr<adm::AudioObject>> Renderer::getDocumentAudioObjects() {
  std::vector<std::shared_ptr<adm::AudioObject>> objects;
  for(auto object : _admDocument->getElements<adm::AudioObject>()) {
    objects.push_back(object);
  }
  return objects;
}

std::shared_ptr<bw64::AxmlChunk> Renderer::getAdmXmlChunk() const {
  return parseAdmXmlChunk(_inputFile);
}

std::shared_ptr<bw64::ChnaChunk> Renderer::getAdmChnaChunk() const {
  return parseAdmChnaChunk(_inputFile);
}

void Renderer::initAudioProgrammeRendering(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
  _renderers.clear();
  const float audioProgrammeGain = getElementGain(formatId(audioProgramme->get<adm::AudioProgrammeId>()));
  for(const std::shared_ptr<adm::AudioContent> audioContent : getAudioContents(audioProgramme)) {
    const float audioContentGain = audioProgrammeGain * getElementGain(formatId(audioContent->get<adm::AudioContentId>()));
    for(const std::shared_ptr<adm::AudioObject> audioObject : getAudioObjects(audioContent)) {
      AudioObjectRenderer renderer(_outputLayout, audioObject, _chnaChunk);
      const float audioObjectGain = audioContentGain * getElementGain(formatId(audioObject->get<adm::AudioObjectId>()));
      renderer.applyUserGain(audioObjectGain);
      std::cout << " >> Add renderer: " << renderer << std::endl;
      _renderers.push_back(renderer);
    }
  }
}

void Renderer::initAudioObjectRendering(const std::shared_ptr<adm::AudioObject>& audioObject) {
  _renderers.clear();
  AudioObjectRenderer renderer(_outputLayout, audioObject, _chnaChunk);
  const float audioObjectGain = getElementGain(formatId(audioObject->get<adm::AudioObjectId>()));
  renderer.applyUserGain(audioObjectGain);
  std::cout << " >> Add renderer: " << renderer << std::endl;
  _renderers.push_back(renderer);
}


void Renderer::processAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
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
  outputFileName << replaceSpecialCharacters(audioProgramme->get<adm::AudioProgrammeName>().get()) << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), _outputLayout.channels().size(), _inputFile->sampleRate(), _inputFile->bitDepth(), chna, axml);

  toFile(outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

void Renderer::processAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject) {
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
  outputFileName << replaceSpecialCharacters(audioObject->get<adm::AudioObjectName>().get()) << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), _outputLayout.channels().size(), _inputFile->sampleRate(), _inputFile->bitDepth(), chna, axml);

  toFile(outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

size_t Renderer::processBlock(const size_t nbFrames, const float* input, float* output) const {
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
  const size_t inputBufferLength = BLOCK_SIZE * _inputNbChannels;
  const size_t outputBufferLength = BLOCK_SIZE * outputNbChannels;

  // Read file, render with gains and write output file
  float inputBuffer[inputBufferLength]; // nb of samples * nb input channels
  memset(inputBuffer, 0.0, inputBufferLength * sizeof(float));

  while (!_inputFile->eof()) {
    // Reset output buffer
    float outputBuffer[outputBufferLength]; // nb of samples * nb output channels
    memset(outputBuffer, 0.0, outputBufferLength * sizeof(float));

    // Read a data block
    auto nbFrames = _inputFile->read(inputBuffer, BLOCK_SIZE);
    processBlock(nbFrames, inputBuffer, outputBuffer);
    outputFile->write(outputBuffer, nbFrames);
  }
  _inputFile->seek(0);
}

}
