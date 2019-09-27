#include <iostream>

#include "adm/common_definitions.hpp"

#include "parser.hpp"

namespace admrenderer {

  std::shared_ptr<adm::Document> parseAdmXml(const std::unique_ptr<bw64::Bw64Reader>& bw64File) {
    // Extract ADM XML from BWF64
    std::cout << std::endl << "### ADM XML from BWF64 file:" << std::endl;
    std::stringstream axmlStringstream;
    if (bw64File->axmlChunk()) {
      bw64File->axmlChunk()->write(axmlStringstream);
      // std::cout << axmlStringstream.str();
    } else {
      std::cerr << "could not find an axml chunk";
      exit(1);
    }

    // Read file ADM composition and write XML data to stdout
    auto admDocument = adm::parseXml(axmlStringstream);
    std::stringstream xmlStream;
    adm::writeXml(xmlStream, admDocument);
    std::cout << xmlStream.str();

    return admDocument;
  }

  std::shared_ptr<bw64::ChnaChunk> parseAdmChunk(const std::unique_ptr<bw64::Bw64Reader>& bw64File) {
    // BWF64 file infos
    std::cout << std::endl << "### BWF64 file infos:" << std::endl;
    std::cout << "Format:" << std::endl;
    std::cout << " - formatTag: " << bw64File->formatTag() << std::endl;
    std::cout << " - channels: " << bw64File->channels() << std::endl;
    std::cout << " - sampleRate: " << bw64File->sampleRate() << std::endl;
    std::cout << " - bitDepth: " << bw64File->bitDepth() << std::endl;
    std::cout << " - numerOfFrames: " << bw64File->numberOfFrames() << std::endl;

    // std::cout << "chunkIds:" << std::endl;
    // for (auto& chunk : bw64File->chunks()) {
    //   std::cout << " - " << '\'' << bw64::utils::fourCCToStr(chunk.id) << '\''
    //             << std::endl;
    // }
    if (bw64File->hasChunk(bw64::utils::fourCC("chna"))) {
      if (auto chnaChunk = bw64File->chnaChunk()) {
        std::cout << "ChnaChunk:" << std::endl;
        std::cout << " - numTracks: " << chnaChunk->numTracks() << std::endl;
        std::cout << " - numUids: " << chnaChunk->numUids() << std::endl;
        std::cout << " - audioIds:" << std::endl;
        std::cout << "   TrackNum audioTrackUID audioTrackFormatID    audioPackFormatID" << std::endl;
        for (auto audioId : chnaChunk->audioIds()) {
          std::cout << "   - ";
          std::cout << audioId.trackIndex() << ", " << audioId.uid() << ", "
                    << audioId.trackRef() << ", " << audioId.packRef()
                    << std::endl;
        }
        return chnaChunk;
      }
    }

    return std::shared_ptr<bw64::ChnaChunk>();
  }


  void parseAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
    // parse audio programmes to get audio contents
    displayAudioProgramme(audioProgramme);
    for(auto audioContent : audioProgramme->getReferences<adm::AudioContent>()) {
      parseAudioContent(audioContent);
    }
  }

  void parseAudioContent(const std::shared_ptr<adm::AudioContent>& audioContent) {
    // parse audio content to get audio objects
    displayAudioContent(audioContent);
    for(auto audioObject : audioContent->getReferences<adm::AudioObject>()) {
      parseAudioObject(audioObject);
    }
  }

  void parseAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject) {
    // parse audio objects to get audio packs and tracks
    displayAudioObject(audioObject);

    for(auto audioTrackUidRef : audioObject->getReferences<adm::AudioTrackUid>()) {
      parseAudioTrackUid(audioTrackUidRef);
    }

    for(auto audioPackFormatRef : audioObject->getReferences<adm::AudioPackFormat>()) {
      parseAudioPackFormat(audioPackFormatRef);
    }
  }

  void parseAudioTrackUid(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid) {
    displayAudioTrackUid(audioTrackUid);
    if(auto audioTrackFormatRef = audioTrackUid->getReference<adm::AudioTrackFormat>()) {
      std::cout << "\t |> ";
      parseAudioTrackFormat(audioTrackFormatRef);
    }
    if(auto audioPackFormatRef = audioTrackUid->getReference<adm::AudioPackFormat>()) {
      std::cout << "\t |> ";
      parseAudioPackFormat(audioPackFormatRef);
    }
  }

  void parseAudioPackFormat(const std::shared_ptr<adm::AudioPackFormat>& audioPackFormat) {
    displayAudioPackFormat(audioPackFormat);
  }

  void parseAudioTrackFormat(const std::shared_ptr<adm::AudioTrackFormat>& audioTrackFormat) {
    displayAudioTrackFormat(audioTrackFormat);
  }


  void displayAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
    const std::string programmeTitle = audioProgramme->get<adm::AudioProgrammeName>().get();
    std::cout << "AudioProgramme: " << formatId(audioProgramme->get<adm::AudioProgrammeId>());
    std::cout << ": " << programmeTitle;
    if(audioProgramme->has<adm::AudioProgrammeLanguage>()) {
      std::cout << " (" << audioProgramme->get<adm::AudioProgrammeLanguage>() << ")";
    }
    std::cout << " [ID: " << audioProgramme->get<adm::AudioProgrammeId>().get<adm::AudioProgrammeIdValue>() << "]"<< std::endl;
  }

  void displayAudioContent(const std::shared_ptr<adm::AudioContent>& audioContent) {
    std::cout << "\tAudioContent: " << formatId(audioContent->get<adm::AudioContentId>())
              << ": " << audioContent->get<adm::AudioContentName>();
    if(audioContent->has<adm::AudioContentLanguage>()) {
      std::cout << " (" << audioContent->get<adm::AudioContentLanguage>() << ")";
    }
    std::cout << " [ID: " << audioContent->get<adm::AudioContentId>().get<adm::AudioContentIdValue>() << "]";
    std::cout << std::endl;
  }

  void displayAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject) {
    const std::string audioObjectName = audioObject->get<adm::AudioObjectName>().get();
    std::cout << "\t\tAudioObject: " << formatId(audioObject->get<adm::AudioObjectId>())
              << ": " << audioObjectName
              << " [ID: " << audioObject->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>() << "]"
              << std::endl;
  }

  void displayAudioTrackUid(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid) {
    std::cout << "\t\t\tAudioTrackUid: " << formatId(audioTrackUid->get<adm::AudioTrackUidId>());
    if(audioTrackUid->has<adm::BitDepth>()) {
      std::cout << " Bitdepth: " << audioTrackUid->get<adm::BitDepth>();
    }
    if(audioTrackUid->has<adm::SampleRate>()) {
      std::cout << " SampleRate: " << audioTrackUid->get<adm::SampleRate>();
    }
    std::cout << " [ID: " << audioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>() << "]";
    std::cout << std::endl;
  }

  void displayAudioPackFormat(const std::shared_ptr<adm::AudioPackFormat>& audioPackFormat) {
    const size_t audioPackFormatId = audioPackFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
    const adm::TypeDescriptor typeDescriptor = audioPackFormat->get<adm::TypeDescriptor>();
    std::cout << "\t\t\tAudioPackFormat: " << formatId(audioPackFormat->get<adm::AudioPackFormatId>())
              << " [ID: " << audioPackFormatId << "] "
              << " => " << audioPackFormat->get<adm::AudioPackFormatName>()
              << " " << adm::formatTypeDefinition(typeDescriptor)
              << std::endl;
  }

  void displayAudioTrackFormat(const std::shared_ptr<adm::AudioTrackFormat>& audioTrackFormat) {
    const size_t audioTrackFormatId = audioTrackFormat->get<adm::AudioTrackFormatId>().get<adm::AudioTrackFormatIdValue>().get();
    const adm::FormatDescriptor formatDescriptor = audioTrackFormat->get<adm::FormatDescriptor>();
    std::cout << "\t\t\tAudioTrackFormat: " << formatId(audioTrackFormat->get<adm::AudioTrackFormatId>())
              << " [ID: " << audioTrackFormatId << "]"
              << " => " << audioTrackFormat->get<adm::AudioTrackFormatName>()
              << " " << adm::formatFormatDefinition(formatDescriptor)
              << std::endl;
  }

  std::vector<std::shared_ptr<adm::AudioObject>> getAudioObjects(const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
    std::vector<std::shared_ptr<adm::AudioObject>> audioObjects;
    for(auto audioContent : audioProgramme->getReferences<adm::AudioContent>()) {
      for(auto audioObjectRef : audioContent->getReferences<adm::AudioObject>()) {
        audioObjects.push_back(audioObjectRef);
      }
    }
    return audioObjects;
  }

  std::vector<std::shared_ptr<adm::AudioTrackUid>> getAudioTrackUids(const std::shared_ptr<adm::AudioObject>& audioObject) {
    std::vector<std::shared_ptr<adm::AudioTrackUid>> audioTrackUids;
    for(auto audioTrackUidRef : audioObject->getReferences<adm::AudioTrackUid>()) {
      audioTrackUids.push_back(audioTrackUidRef);
    }
    return audioTrackUids;
  }
  std::vector<std::shared_ptr<adm::AudioPackFormat>> getAudioPackFormats(const std::shared_ptr<adm::AudioObject>& audioObject) {
    std::vector<std::shared_ptr<adm::AudioPackFormat>> audioPackFormats;
    for(auto audioPackFormatRef : audioObject->getReferences<adm::AudioPackFormat>()) {
      audioPackFormats.push_back(audioPackFormatRef);
    }
    return audioPackFormats;
  }

}
