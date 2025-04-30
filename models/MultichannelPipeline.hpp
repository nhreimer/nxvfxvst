#pragma once

#include "data/PipelineContext.hpp"
#include "models/ChannelPipeline.hpp"
#include "models/encoder/EncoderFactory.hpp"
#include "shapes/TimedMessage.hpp"

#ifdef BUILD_PLUGIN
#include "vst/version.h"
#else
#include "app/version.hpp"
#endif

namespace nx
{
  class MultichannelPipeline
  {

    // TODO: This is hardcoded for now, but it should be adjustable by the user
    static constexpr int MAX_CHANNELS = 4;

  public:
    explicit MultichannelPipeline( PipelineContext& context );
    ~MultichannelPipeline() = default;

    [[nodiscard]]
    nlohmann::json saveState() const;
    void restoreState( const nlohmann::json &j ) const;

    void processMidiEvent( const Midi_t &midi ) const;

    void draw(sf::RenderWindow &window);
    void drawMenu();

    void update(const sf::Time &deltaTime) const;

  private:

    PipelineContext m_ctx;

    std::array< std::unique_ptr< ChannelPipeline >, MAX_CHANNELS > m_channels;
    TimedMessage m_messageClock;

    int m_selectedChannel { 0 };

    E_Encoder m_encoderType { E_Encoder::E_RawRGBA };
    EncoderData_t m_encoderData {};

    // used for saving to video
    std::unique_ptr< IEncoder > m_encoder;

    int32_t m_selectedCodec = 0;

  };

} // namespace nx
