#pragma once

#include <ear/ear.hpp>
#include <adm/adm.hpp>

namespace admrenderer {

class AudioObjectRenderer {

public:
  AudioObjectRenderer(const ear::Layout& outputLayout,
                      const std::shared_ptr<adm::AudioObject>& audioObject);

  std::vector<size_t> getTrackMapping(const size_t& outputTrackId) const;
  void addTrackToMapping(const size_t& outputTrackId, const size_t& inputTrackId);

  float getTrackGain(const size_t& outputTrackId) const;
  void setTrackGain(const size_t& outputTrackId, const float& gain);
  void applyGain(const size_t& outputTrackId, const float& gain);

  size_t getNbOutputTracks() const;

  void renderAudioFrame(const float* in, float* out);

private:
  void setDirectSpeakerGains();
  void init();

  void checkAudioPackFormatId(const adm::AudioPackFormatId& audioPackFormatId, const size_t nbAudioTracks);

  friend std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer);

private:
  const ear::Layout _outputLayout;
  const std::shared_ptr<adm::AudioObject> _audioObject;
  /// Map of input channels (values) by output channels (keys)
  std::map<size_t, std::vector<size_t>> _trackMapping;
  /// Map of gains to apply to input channels (values) by output channels (keys)
  std::map<size_t, float> _trackGains;
};

std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer);

}
