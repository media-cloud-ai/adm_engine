#pragma once

#include <ear/ear.hpp>
#include <bw64/bw64.hpp>
#include <adm/adm.hpp>

#include "audio_object_renderer.hpp"

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

namespace admrenderer {

const unsigned int BLOCK_SIZE = 4096; // in frames

void render(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
            const std::string& outputLayout,
            const std::string& outputDirectory,
            const float dialogGain);
void renderToFile(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
            const std::vector<AudioObjectRenderer>& renderers,
            const std::unique_ptr<bw64::Bw64Writer>& outputFile);

void renderAudioProgramme(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                          const std::shared_ptr<adm::AudioProgramme>& audioProgramme,
                          const ear::Layout& outputLayout,
                          const std::string& outputDirectory);

void renderAudioObject(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
                       const std::shared_ptr<adm::AudioObject>& audioObject,
                       const ear::Layout& outputLayout,
                       const std::string& outputDirectory);

void checkAudioPackFormatId(const adm::AudioPackFormatId& audioPackFormatId, const size_t nbAudioTracks);

std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioProgramme>& audioProgramme, const ear::Layout& outputLayout);
std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioObject>& audioObject, const ear::Layout& outputLayout);

std::shared_ptr<adm::AudioObject> createAdmAudioObject(const adm::AudioObjectName& audioObjectName, const ear::Layout& outputLayout);

std::shared_ptr<bw64::AxmlChunk> createAxmlChunk(const std::shared_ptr<adm::Document>& admDocument);
std::shared_ptr<bw64::ChnaChunk> createChnaChunk(const std::shared_ptr<adm::Document>& admDocument);

template<class T>
std::string toString(const std::shared_ptr<T>& admElement) {
  std::stringstream ss;
  admElement->print(ss);
  return ss.str();
}

}
