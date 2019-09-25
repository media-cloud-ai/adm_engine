#include <algorithm>

#include "ear/ear.hpp"
#include "bw64/bw64.hpp"
#include "adm/adm.hpp"

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

namespace admrenderer {

const unsigned int BLOCK_SIZE = 4096; // in frames


class AudioObjectRenderer {

public:
  AudioObjectRenderer(const std::vector<size_t>& trackIds,
                      const size_t& audioPackFormat,
                      const adm::TypeDescriptor& typeDescriptor);

  std::vector<size_t> getTrackIds() const;
  size_t getAudioPackFormat() const;
  adm::TypeDescriptor getTypeDescriptor() const;
  float getTrackGain(const size_t trackId) const;
  void setTrackGain(const size_t trackId, const float gain);

private:
  const std::vector<size_t> _trackIds;
  const size_t _audioPackFormat;
  const adm::TypeDescriptor _typeDescriptor;
  std::map<size_t, float> _trackGains;
};

std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer) {
  os << "Channels: ";
  for (size_t channel : renderer.getTrackIds()) {
     os << channel << " ";
  }
  os << " - AudioPackFormat: " << renderer.getAudioPackFormat();
  os << " - TypeDesciptor: " << adm::formatTypeDefinition(renderer.getTypeDescriptor());
  return os;
}

std::vector<float> getDirectSpeakersGains(const ear::Layout& layout, const size_t& outputNbChannels);

void render(const std::unique_ptr<bw64::Bw64Reader>& bw64File,
            const ear::Layout& layout,
            const std::string programmeTitle,
            const std::string outputDirectory,
            const std::vector<AudioObjectRenderer> renderers);


}
