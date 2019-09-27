#include <algorithm>
#include <iomanip>

#include <adm/utilities/id_assignment.hpp>
#include <adm/common_definitions.hpp>

#include "renderer.hpp"
#include "parser.hpp"

namespace admrenderer {

void render(const std::unique_ptr<bw64::Bw64Reader>& inputFile, const std::string& outputLayout, const std::string& outputDirectory, const float dialogGain) {
  auto admDocument = getAdmDocument(parseAdmXmlChunk(inputFile));
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
      // parseAudioObject(audioObject);
      renderAudioObject(inputFile, audioObject, layout, outputDirectory);
    }
    return;
  }

  auto chnaChunk = parseAdmChnaChunk(inputFile);
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

std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioProgramme>& audioProgramme, const ear::Layout& outputLayout) {
  std::shared_ptr<adm::Document> admDocument = adm::Document::create();
  auto admProgramme = adm::AudioProgramme::create(audioProgramme->get<adm::AudioProgrammeName>());
  auto mixContent = adm::AudioContent::create(adm::AudioContentName("Mix"));
  auto mixObject = createAdmAudioObject(adm::AudioObjectName("Mix"), outputLayout);
  mixContent->addReference(mixObject);
  admProgramme->addReference(mixContent);
  admDocument->add(admProgramme);
  // adm::reassignIds(admDocument);
  return admDocument;
}

std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioObject>& audioObject, const ear::Layout& outputLayout) {
  std::shared_ptr<adm::Document> admDocument = adm::Document::create();
  auto mixObject = createAdmAudioObject(audioObject->get<adm::AudioObjectName>(), outputLayout);
  admDocument->add(mixObject);
  // adm::reassignIds(admDocument);
  return admDocument;
}

std::shared_ptr<adm::AudioObject> createAdmAudioObject(const adm::AudioObjectName& audioObjectName, const ear::Layout& outputLayout) {
  auto mixObject = adm::AudioObject::create(audioObjectName);
  auto mixPackFormat = adm::AudioPackFormat::create(adm::AudioPackFormatName(""), adm::TypeDefinition::DIRECT_SPEAKERS);
  adm::AudioPackFormatId mixPackFormatId = adm::audioPackFormatLookupTable().at(outputLayout.name());
  mixPackFormat->set(mixPackFormatId);

  mixObject->addReference(mixPackFormat);
  for (auto channel : outputLayout.channels())
  {
    auto audioTrackUid = adm::AudioTrackUid::create();
    audioTrackUid->setReference(mixPackFormat);

    adm::AudioTrackFormatId audioTrackFormatId = adm::audioTrackFormatLookupTable().at(channel.name());
    auto audioTrackFormat = adm::AudioTrackFormat::create(adm::AudioTrackFormatName(""), adm::FormatDefinition::PCM);
    audioTrackFormat->set(audioTrackFormatId);
    audioTrackUid->setReference(audioTrackFormat);

    mixObject->addReference(audioTrackUid);
  }
  return mixObject;
}

std::shared_ptr<bw64::AxmlChunk> createAxmlChunk(const std::shared_ptr<adm::Document>& admDocument) {
  std::stringstream xmlStream;
  adm::writeXml(xmlStream, admDocument);
  return std::shared_ptr<bw64::AxmlChunk>(new bw64::AxmlChunk(xmlStream.str()));
}

std::shared_ptr<bw64::ChnaChunk> createChnaChunk(const std::shared_ptr<adm::Document>& admDocument) {
  std::vector<bw64::AudioId> audioIds;

  auto audioObjects = admDocument->getElements<adm::AudioObject>();
  for(auto audioObject : audioObjects) {
    for(auto audioTrackUid : audioObject->getReferences<adm::AudioTrackUid>()) {
      audioIds.push_back(bw64::AudioId(audioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get(),
                                       formatId(audioTrackUid->get<adm::AudioTrackUidId>()),
                                       formatId(audioTrackUid->getReference<adm::AudioTrackFormat>()->get<adm::AudioTrackFormatId>()),
                                       formatId(audioTrackUid->getReference<adm::AudioPackFormat>()->get<adm::AudioPackFormatId>())
                                      ));
    }
  }
  return std::shared_ptr<bw64::ChnaChunk>(new bw64::ChnaChunk(audioIds));
}

void renderAudioProgramme(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                          const std::shared_ptr<adm::AudioProgramme>& audioProgramme,
                          const ear::Layout& outputLayout,
                          const std::string& outputDirectory) {
  std::cout << "### Render audio programme: " << toString(audioProgramme) << std::endl;

  std::vector<AudioObjectRenderer> renderers;
  for(const std::shared_ptr<adm::AudioObject> audioObject : getAudioObjects(audioProgramme)) {
    AudioObjectRenderer renderer(outputLayout, audioObject);
    std::cout << " >> Add renderer: " << renderer << std::endl;
    renderers.push_back(renderer);
  }

  // Create output programme ADM
  std::shared_ptr<adm::Document> document = createAdmDocument(audioProgramme, outputLayout);
  std::shared_ptr<bw64::AxmlChunk> axml = createAxmlChunk(document);
  std::shared_ptr<bw64::ChnaChunk> chna = createChnaChunk(document);

  // Output file
  std::stringstream outputFileName;
  outputFileName << outputDirectory;
  if(outputDirectory.back() != std::string(PATH_SEPARATOR).back()) {
    outputFileName << PATH_SEPARATOR;
  }
  outputFileName << audioProgramme->get<adm::AudioProgrammeName>().get() << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), outputLayout.channels().size(), inputFile->sampleRate(), inputFile->bitDepth(), chna, axml);

  renderToFile(inputFile, renderers, outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

void renderAudioObject(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                          const std::shared_ptr<adm::AudioObject>& audioObject,
                          const ear::Layout& outputLayout,
                          const std::string& outputDirectory) {
  std::cout << "### Render audio object: " << toString(audioObject) << std::endl;

  std::vector<AudioObjectRenderer> renderers;
  AudioObjectRenderer renderer(outputLayout, audioObject);
  std::cout << " >> Add renderer: " << renderer << std::endl;
  renderers.push_back(renderer);

  // Create output programme ADM
  std::shared_ptr<adm::Document> document = createAdmDocument(audioObject, outputLayout);
  std::shared_ptr<bw64::AxmlChunk> axml = createAxmlChunk(document);
  std::shared_ptr<bw64::ChnaChunk> chna = createChnaChunk(document);

  // Output file
  std::stringstream outputFileName;
  outputFileName << outputDirectory;
  if(outputDirectory.back() != std::string(PATH_SEPARATOR).back()) {
    outputFileName << PATH_SEPARATOR;
  }
  outputFileName << audioObject->get<adm::AudioObjectName>().get() << ".wav";
  std::unique_ptr<bw64::Bw64Writer> outputFile =
    bw64::writeFile(outputFileName.str(), outputLayout.channels().size(), inputFile->sampleRate(), inputFile->bitDepth(), chna, axml);

  renderToFile(inputFile, renderers, outputFile);
  std::cout << " >> Done: " << outputFileName.str() << std::endl;
}

}
