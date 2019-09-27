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

class Renderer {

public:
  Renderer(const std::unique_ptr<bw64::Bw64Reader>& inputFile,
           const std::string& outputLayout,
           const std::string& outputDirectory,
           const float dialogGain);

  void process();
  void processAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  void processAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject);

  size_t processBlock(const size_t nbFrames,
                      const float* input,
                      float* output);

  void toFile(const std::unique_ptr<bw64::Bw64Writer>& outputFile);

private:
  const std::unique_ptr<bw64::Bw64Reader>& _inputFile;
  const size_t _inputNbChannels;
  const ear::Layout _outputLayout;
  const std::string _outputDirectory;
  const float _dialogGain;

  std::vector<AudioObjectRenderer> _renderers;
};

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
