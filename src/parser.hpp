#pragma once

#include <adm/parse.hpp>
#include <adm/write.hpp>

#include <bw64/bw64.hpp>

namespace admrenderer {

  std::shared_ptr<adm::Document> parseAdmXml(const std::unique_ptr<bw64::Bw64Reader>& bw64File);
  std::shared_ptr<bw64::ChnaChunk> parseAdmChunk(const std::unique_ptr<bw64::Bw64Reader>& bw64File);

  void parseAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  void parseAudioContent(const std::shared_ptr<adm::AudioContent>& audioContent);
  void parseAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject);
  void parseAudioTrackUid(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid);
  void parseAudioPackFormat(const std::shared_ptr<adm::AudioPackFormat>& audioPackFormat);
  void parseAudioTrackFormat(const std::shared_ptr<adm::AudioTrackFormat>& audioTrackFormat);

  void displayAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  void displayAudioContent(const std::shared_ptr<adm::AudioContent>& audioContent);
  void displayAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject);
  void displayAudioTrackUid(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid);
  void displayAudioPackFormat(const std::shared_ptr<adm::AudioPackFormat>& audioPackFormat);
  void displayAudioTrackFormat(const std::shared_ptr<adm::AudioTrackFormat>& audioTrackFormat);


  std::vector<std::shared_ptr<adm::AudioObject>> getAudioObjects(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  std::vector<std::shared_ptr<adm::AudioTrackUid>> getAudioTrackUids(const std::shared_ptr<adm::AudioObject>& audioObject);
  std::vector<std::shared_ptr<adm::AudioPackFormat>> getAudioPackFormats(const std::shared_ptr<adm::AudioObject>& audioObject);

}
