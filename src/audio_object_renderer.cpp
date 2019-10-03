
#include "audio_object_renderer.hpp"

#include "parser.hpp"

#include <adm/common_definitions.hpp>

namespace admrenderer {

AudioObjectRenderer::AudioObjectRenderer(const ear::Layout& outputLayout,
                      const std::shared_ptr<adm::AudioObject>& audioObject,
                      const std::shared_ptr<bw64::ChnaChunk>& chnaChunk)
  : _outputLayout(outputLayout)
  , _audioObject(audioObject)
  , _chnaChunk(chnaChunk)
{
  init();
}

float AudioObjectRenderer::getTrackGain(const size_t& inputTrackId, const size_t& outputTrackId) const {
  return _inputTrackGains.at(inputTrackId).at(outputTrackId);
}

void AudioObjectRenderer::applyUserGain(const float& gain) {
  for(auto& kv : _inputTrackGains) {
    for (int i = 0; i < kv.second.size(); ++i) {
      _inputTrackGains[kv.first][i] *= gain;
    }
  }
}

void AudioObjectRenderer::applyGain(const size_t& inputTrackId, const size_t& outputTrackId, const float& gain) {
  _inputTrackGains[inputTrackId][outputTrackId] *= gain;
}

size_t AudioObjectRenderer::getNbOutputTracks() const {
  return _outputLayout.channels().size();
}

void AudioObjectRenderer::renderAudioFrame(const float* inputFrame, float* outputFrame) {
  for (int oc = 0; oc < getNbOutputTracks(); ++oc) {
  // for each output channel, apply computed gain to input channels...
    for(const size_t ic : _inputTrackIds) { // getTrackMapping(oc)
      outputFrame[oc] += inputFrame[ic] * getTrackGain(ic, oc);
    }
  }
}

std::string AudioObjectRenderer::getAudioTrackFormatIdSpeakerLabel(const adm::AudioTrackFormatId& audioTrackFormatId) {
  std::map<std::string, adm::AudioTrackFormatId> audioTrackFormatBySpeakerLabelMap = adm::audioTrackFormatLookupTable();
  for(auto& entry : audioTrackFormatBySpeakerLabelMap) {
    if(entry.second == audioTrackFormatId) {
      std::cout << "Found AudioTrackFormatId " << adm::formatId(audioTrackFormatId) << " speaker label: " << entry.first << std::endl;
      return entry.first;
    }
  }
  throw std::runtime_error("No speaker label found.");
}

std::string AudioObjectRenderer::getAudioTrackSpeakerLabel(const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid) {
  std::shared_ptr<adm::AudioTrackFormat> audioTrackFormat = audioTrackUid->getReference<adm::AudioTrackFormat>();
  if(audioTrackFormat) {
    adm::AudioTrackFormatId audioTrackFormatId = audioTrackFormat->get<adm::AudioTrackFormatId>();
    return getAudioTrackFormatIdSpeakerLabel(audioTrackFormatId);
  } else if(_chnaChunk) {
    std::cout << "[WARNING] No AudioTrackFormat into ADM XML. Check into CHNA chunk content..." << std::endl;
    std::string audioTrackUidStr = adm::formatId(audioTrackUid->get<adm::AudioTrackUidId>());
    for (auto audioId : _chnaChunk->audioIds()) {
      if(audioTrackUidStr == audioId.uid()) {
        adm::AudioTrackFormatId audioTrackFormatId = adm::parseAudioTrackFormatId(audioId.trackRef());
        return getAudioTrackFormatIdSpeakerLabel(audioTrackFormatId);
      }
    }
  }
  throw std::runtime_error("Not enough content to find speaker label.");
}

void AudioObjectRenderer::setDirectSpeakerTrackGains(const adm::AudioPackFormatId& audioPackFormatId, const std::shared_ptr<adm::AudioTrackUid>& audioTrackUid) {

  // Add input track id and init gains
  const size_t inputTrackId = audioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get() - 1;
  _inputTrackIds.push_back(inputTrackId);
  _inputTrackGains[inputTrackId] = std::vector<float>(getNbOutputTracks());
  for (int i = 0; i < getNbOutputTracks(); ++i) {
    _inputTrackGains[inputTrackId][i] = 1.0;
  }

  // Get speaker label
  const std::string audioPackFormatIdValue = adm::formatId(audioPackFormatId);
  const std::string speakerLabel = getAudioTrackSpeakerLabel(audioTrackUid);

  // calculate gains for direct speakers
  std::cout << "Compute direct speaker gains for AudioPackFormat: " << audioPackFormatIdValue << ", and speaker label: " << speakerLabel << std::endl;
  ear::GainCalculatorDirectSpeakers speakerGainCalculator(_outputLayout);
  ear::DirectSpeakersTypeMetadata speakersTypeMetadata;
  speakersTypeMetadata.audioPackFormatID = audioPackFormatIdValue;
  speakersTypeMetadata.speakerLabels.push_back(speakerLabel);

  std::vector<float> gains(getNbOutputTracks());
  speakerGainCalculator.calculate(speakersTypeMetadata, gains);
  for (int i = 0; i < gains.size(); ++i) {
    applyGain(inputTrackId, i, gains[i]);
  }
}

void AudioObjectRenderer::init() {
  for(auto audioPackFormat : getAudioPackFormats(_audioObject)) {
    const adm::TypeDescriptor typeDescriptor = audioPackFormat->get<adm::TypeDescriptor>();
    switch(typeDescriptor.get()) {
      case 1: // TypeDefinition::DIRECT_SPEAKERS
        break;
      case 0: // TypeDefinition::UNDEFINED
      case 2: // TypeDefinition::MATRIX
      case 3: // TypeDefinition::OBJECTS
      case 4: // TypeDefinition::HOA
      case 5: // TypeDefinition::BINAURAL
      default:
        std::cerr << "Unsupported type descriptor: " << adm::formatTypeDefinition(typeDescriptor) << std::endl;
        exit(1);
    }

    // Render to direct speaker:
    std::vector<std::shared_ptr<adm::AudioTrackUid>> audioTrackUids = getAudioTrackUids(_audioObject);
    const adm::AudioPackFormatId audioPackFormatId = audioPackFormat->get<adm::AudioPackFormatId>();
    for(auto audioTrackUid : audioTrackUids) {
      setDirectSpeakerTrackGains(audioPackFormatId, audioTrackUid);
    }
    checkAudioPackFormatId(audioPackFormatId, audioTrackUids.size());
  }
}

void AudioObjectRenderer::checkAudioPackFormatId(const adm::AudioPackFormatId& audioPackFormatId, const size_t nbAudioTracks) {
  size_t expectedNbTracks = 0;
  switch(audioPackFormatId.get<adm::AudioPackFormatIdValue>().get()) {
    case 0x01: expectedNbTracks = 1; break;
    case 0x02: expectedNbTracks = 2; break;
    // case 0x0A: expectedNbTracks = 3; break;
    // case 0x0B: expectedNbTracks = 4; break;
    // case 0x0C: expectedNbTracks = 5; break;
    // case 0x03: expectedNbTracks = 6; break;
    default:
      // See ITU-R BS.2094-1: Common definitions for the Audio Definition Model
      std::cerr << "AudioPackFormat not supported yet: " << adm::formatId(audioPackFormatId) << std::endl;
      exit(1);
      break;
  }

  if(expectedNbTracks != nbAudioTracks) {
    std::cerr << "AudioPackFormat does not fit the number of tracks tracks: " << nbAudioTracks << std::endl;
    exit(1);
  }
}


std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer) {
  os << "Gains: ";
  for (const size_t inputTrackId : renderer._inputTrackIds) {
    os << "{ input track " << inputTrackId << ": " ;
    std::vector<float> gains = renderer._inputTrackGains.at(inputTrackId);
    for (int oc = 0; oc < gains.size(); ++oc) {
      os << "{ oc: " << oc << " => gain: " << gains.at(oc) << " } ";
    }
    os << "} ";
  }
  return os;
}

}
