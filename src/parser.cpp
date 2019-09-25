#include <iostream>

#include "parser.hpp"

namespace admrenderer {

  std::shared_ptr<adm::Document> parseAdmXml(const std::unique_ptr<bw64::Bw64Reader>& bw64File) {
    // Extract ADM XML from BWF64
    std::cout << std::endl << "### Extract ADM XML from BWF64:" << std::endl;
    std::stringstream axmlStringstream;
    if (bw64File->axmlChunk()) {
      bw64File->axmlChunk()->write(axmlStringstream);
      // std::cout << axmlStringstream.str();
    } else {
      std::cerr << "could not find an axml chunk";
      exit(1);
    }

    // Read file ADM composition
    std::cout << std::endl << "### Read ADM composition:" << std::endl;
    auto admDocument = adm::parseXml(axmlStringstream);

    // write XML data to stdout
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

}
