#pragma once

#include <queue>

#include "data/PipelineContext.hpp"
#include "helpers/Definitions.hpp"
#include "models/ChannelPipeline.hpp"
#include "models/encoder/EncoderFactory.hpp"
#include "shapes/TimedMessage.hpp"

#include "utils/ThreadPool.hpp"

#ifdef BUILD_PLUGIN
#include "vst/version.h"
#else
#include "app/version.hpp"
#endif

namespace nx
{
  class MultichannelPipeline
  {

    struct ChannelDrawingData_t
    {
      int32_t priority { 0 };
      const sf::RenderTexture * texture { nullptr };
      const sf::BlendMode * blendMode { nullptr };

      // Overload '<' for std::priority_queue (max-heap)
      // Lower priority value = higher actual priority
      bool operator<(const ChannelDrawingData_t& other) const
      {
        return priority > other.priority;
      }

      bool operator>(const ChannelDrawingData_t& other) const
      {
        return priority < other.priority;
      }
    };

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

    std::priority_queue< ChannelDrawingData_t > m_drawingPrioritizer;

  };

} // namespace nx
