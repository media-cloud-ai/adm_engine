
#include "audio_object_renderer.hpp"

#include "parser.hpp"

namespace admrenderer {

AudioObjectRenderer::AudioObjectRenderer(const ear::Layout& outputLayout,
                      const std::shared_ptr<adm::AudioObject>& audioObject)
  : _outputLayout(outputLayout)
  , _audioObject(audioObject)
{
  for (int i = 0; i < getNbOutputTracks(); ++i)
  {
    _trackMapping[i] = std::vector<size_t>();
    _trackGains[i] = 1.0;
  }
  init();
}

std::vector<size_t> AudioObjectRenderer::getTrackMapping(const size_t& outputTrackId) const {
  return _trackMapping.at(outputTrackId);
}
void AudioObjectRenderer::addTrackToMapping(const size_t& outputTrackId, const size_t& inputTrackIds) {
  _trackMapping[outputTrackId].push_back(inputTrackIds);
}

float AudioObjectRenderer::getTrackGain(const size_t& outputTrackId) const {
  return _trackGains.at(outputTrackId);
}

void AudioObjectRenderer::setTrackGain(const size_t& outputTrackId, const float& gain) {
  _trackGains[outputTrackId] = gain;
}

void AudioObjectRenderer::applyGain(const size_t& outputTrackId, const float& gain) {
  _trackGains[outputTrackId] *= gain;
}

size_t AudioObjectRenderer::getNbOutputTracks() const {
  return _outputLayout.channels().size();
}

void AudioObjectRenderer::renderAudioFrame(const float* inputFrame, float* outputFrame) {
  for (int oc = 0; oc < getNbOutputTracks(); ++oc) {
  // for each output channel, apply the mapped input channels...
    for(const size_t ic : getTrackMapping(oc)) {
      outputFrame[oc] += inputFrame[ic] * getTrackGain(oc);
    }
  }
}

void AudioObjectRenderer::setDirectSpeakerGains() {

  // calculate gains for direct speakers
  ear::GainCalculatorDirectSpeakers speakerGainCalculator(_outputLayout);
  ear::DirectSpeakersTypeMetadata speakersMetadata;

  std::vector<float> gains(getNbOutputTracks());
  speakerGainCalculator.calculate(speakersMetadata, gains);
  for (int i = 0; i < gains.size(); ++i) {
    applyGain(i, gains[i]);
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
    setDirectSpeakerGains();
    std::vector<std::shared_ptr<adm::AudioTrackUid>> audioTrackUids = getAudioTrackUids(_audioObject);

    const adm::AudioPackFormatId audioPackFormatId = audioPackFormat->get<adm::AudioPackFormatId>();
    checkAudioPackFormatId(audioPackFormatId, audioTrackUids.size());

    switch(audioPackFormatId.get<adm::AudioPackFormatIdValue>().get()) {
      case 1: // AP_00010001 => urn:itu:bs:775:3:pack:mono_(0+1+0)
        for (int oc = 0; oc < getNbOutputTracks(); ++oc) {
          addTrackToMapping(oc, audioTrackUids.at(0)->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get() - 1);
        }
        break;

      case 2: // AP_00010002 => urn:itu:bs:2051:0:pack:stereo_(0+2+0)
        for (int oc = 0; oc < getNbOutputTracks(); ++oc) {
          addTrackToMapping(oc, audioTrackUids.at(oc)->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get() - 1);
        }
        break;

      default:
        std::cerr << "Unsupported audio pack format: " << adm::formatId(audioPackFormatId) << std::endl;
        exit(1);
    }
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
  os << "Mapping: ";
  for (const auto& kv : renderer._trackMapping) {
    os << "{ " << kv.first << " <= ";
    for(const size_t ic : kv.second) {
      os << ic << " ";
    }
    os << "} ";
  }
  os << " - Gains: ";
  for (const auto& kv : renderer._trackGains) {
    os << "{ " << kv.first << " : " << kv.second << " } ";
  }
  return os;
}

}
