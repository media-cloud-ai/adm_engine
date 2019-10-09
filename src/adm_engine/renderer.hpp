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
           const std::string& outputDirectory = ".",
           const float dialogGain = 1.0);

  void process();

  void initAudioProgrammeRendering(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  void initAudioObjectRendering(const std::shared_ptr<adm::AudioObject>& audioObject);

  void processAudioProgramme(const std::shared_ptr<adm::AudioProgramme>& audioProgramme);
  void processAudioObject(const std::shared_ptr<adm::AudioObject>& audioObject);

  size_t processBlock(const size_t nbFrames,
                      const float* input,
                      float* output) const;

  void toFile(const std::unique_ptr<bw64::Bw64Writer>& outputFile);

  size_t getNbOutputChannels() const { return _outputLayout.channels().size(); }

  std::shared_ptr<adm::Document> getDocument() const { return _admDocument; };
  std::vector<std::shared_ptr<adm::AudioProgramme>> getDocumentAudioProgrammes();
  std::vector<std::shared_ptr<adm::AudioObject>> getDocumentAudioObjects();

  std::shared_ptr<bw64::AxmlChunk> getAdmXmlChunk() const;
  std::shared_ptr<bw64::ChnaChunk> getAdmChnaChunk() const;

private:
  const std::unique_ptr<bw64::Bw64Reader>& _inputFile;
  const size_t _inputNbChannels;
  const ear::Layout _outputLayout;
  const std::string _outputDirectory;
  const float _dialogGain;

  std::shared_ptr<adm::Document> _admDocument;
  std::shared_ptr<bw64::ChnaChunk> _chnaChunk;
  std::vector<AudioObjectRenderer> _renderers;
};

template<class T>
std::string toString(const std::shared_ptr<T>& admElement) {
  std::stringstream ss;
  admElement->print(ss);
  return ss.str();
}

}
