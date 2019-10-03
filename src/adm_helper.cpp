
#include "adm_helper.hpp"

namespace admrenderer {

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

}
