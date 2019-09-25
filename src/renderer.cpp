#include <algorithm>
#include <iomanip>

#include "renderer.hpp"
#include "parser.hpp"

namespace admrenderer {

void render(const std::unique_ptr<bw64::Bw64Reader>& inputFile, const std::string& outputLayout, const std::string& outputDirectory, const float dialogGain) {
  auto admDocument = parseAdmXml(inputFile);
  const ear::Layout layout = ear::getLayout(outputLayout);

  /// Based on Rec. ITU-R  BS.2127-0, 5.2 Determination of Rendering Items (Fig. 3)
  auto audioProgrammes = admDocument->getElements<adm::AudioProgramme>();
  if(audioProgrammes.size()) {
    // // TODO: get audio contents from audio programmes
    for(auto audioProgramme : audioProgrammes) {
      // parseAudioProgramme(audioProgramme);
      renderAudioProgramme(inputFile, audioProgramme, layout, outputDirectory);
    }
    return;
  }

  auto audioObjects = admDocument->getElements<adm::AudioObject>();
  if(audioObjects.size()) {
    // // TODO: parse audio objects for AudioPackFormatRef, AudioTrackUIDRef and nested AudioObjects
    for(auto audioObject : audioObjects) {
      parseAudioObject(audioObject);
      // renderAudioObject(audioObject);
    }
    return;
  }

  auto chnaChunk = parseAdmChunk(inputFile);
  if (chnaChunk) {
    std::vector<bw64::AudioId> audioIds = chnaChunk->audioIds();
    if(audioIds.size()) {
      // TODO: parse to get AudioTrackUID, AudioPackFormat and AudioTrackFormat, and render
      return;
    }
  }
}

void renderToFile(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
            const std::vector<AudioObjectRenderer>& renderers,
            const std::unique_ptr<bw64::Bw64Writer>& outputFile) {

  // Buffers
  const size_t inputNbChannels = inputFile->channels();
  const size_t outputNbChannels = outputFile->channels();
  std::vector<float> fileInputBuffer(BLOCK_SIZE * inputNbChannels);
  std::vector<float> fileOutputBuffer;

  // Read file, render with gains and write output file
  size_t filePosition = 0;
  while (!inputFile->eof()) {
    // Read a data block
    auto readFrames = inputFile->read(&fileInputBuffer[0], BLOCK_SIZE);
    float* ocsample = new float[outputNbChannels];
    size_t frame = 0;
    size_t sample = 0;

    while(frame < readFrames) {
      for (int oc = 0; oc < outputNbChannels; ++oc) {
        // for each output channel, apply the mapped input channels...
        ocsample[oc] = 0.0;
        for(const AudioObjectRenderer render : renderers) {
          for(const size_t ic : render.getTrackMapping(oc)) {
            ocsample[oc] += fileInputBuffer[sample + ic] * render.getTrackGain(oc);
          }
        }
      }
      filePosition += inputNbChannels;
      sample += inputNbChannels;
      for (int oc = 0; oc < outputNbChannels; ++oc) {
        fileOutputBuffer.push_back(ocsample[oc]); // FIXME : double sound in output ?
      }
      frame++;
    }

    auto wroteFrames = outputFile->write(&fileOutputBuffer[0], readFrames);
    fileOutputBuffer.clear();
    delete[] ocsample;
  }
  inputFile->seek(0);
}

void renderAudioProgramme(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                          const std::shared_ptr<adm::AudioProgramme>& audioProgramme,
                          const ear::Layout& outputLayout,
                          const std::string& outputDirectory) {
  std::cout << "Render audio programme: " << toString(audioProgramme) << std::endl;

  std::vector<AudioObjectRenderer> renderers;
  for(const std::shared_ptr<adm::AudioObject> audioObject : getAudioObjects(audioProgramme)) {
    AudioObjectRenderer renderer(outputLayout, audioObject);
    std::cout << " >> Add renderer: " << renderer << std::endl;
    renderers.push_back(renderer);
  }

  // Output file
  std::stringstream outputFileName;
  outputFileName << outputDirectory;
  if(outputDirectory.back() != std::string(PATH_SEPARATOR).back()) {
    outputFileName << PATH_SEPARATOR;
  }
  outputFileName << audioProgramme->get<adm::AudioProgrammeName>().get() << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile = bw64::writeFile(outputFileName.str(), outputLayout.channels().size(), inputFile->sampleRate(), inputFile->bitDepth()/*, chna, axml*/);

  renderToFile(inputFile, renderers, outputFile);
  std::cout << "/// DONE: " << outputFileName.str() << std::endl;
}

void renderAudioObject(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                          const std::shared_ptr<adm::AudioObject>& audioObject,
                          const ear::Layout& outputLayout,
                          const std::string& outputDirectory) {
  // TODO
}

}
