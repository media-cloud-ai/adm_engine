#pragma once

#include <ear/ear.hpp>
#include <adm/adm.hpp>
#include <bw64/bw64.hpp>

namespace admrenderer {

class AudioObjectRenderer {

public:
  AudioObjectRenderer(const ear::Layout& outputLayout,
                      const std::shared_ptr<adm::AudioObject>& audioObject,
                      const std::shared_ptr<bw64::ChnaChunk>& chnaChunk);

  float getTrackGain(const size_t& inputTrackId, const size_t& outputTrackId) const;
  void setTrackGain(const size_t& inputTrackId, const size_t& outputTrackId, const float& gain);
  void applyGain(const size_t& inputTrackId, const size_t& outputTrackId, const float& gain);

  size_t getNbOutputTracks() const;

  void renderAudioFrame(const float* in, float* out);

private:
  std::string getAudioTrackFormatIdSpeakerLabel(const adm::AudioTrackFormatId& audioTrackFormatId);
  std::string getAudioTrackSpeakerLabel(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid);
  void setDirectSpeakerTrackGains(const adm::AudioPackFormatId& audioPackFormatId, const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid);
  void init();

  void checkAudioPackFormatId(const adm::AudioPackFormatId& audioPackFormatId, const size_t nbAudioTracks);

  friend std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer);

private:
  const ear::Layout _outputLayout;
  const std::shared_ptr<adm::AudioObject> _audioObject;
  const std::shared_ptr<bw64::ChnaChunk> _chnaChunk;

  /// Index of input channels
  std::vector<size_t> _inputTrackIds;
  /// Output channel gains (values) by input channel indexes (keys)
  std::map<size_t, std::vector<float>> _inputTrackGains;
};

std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer);

}
