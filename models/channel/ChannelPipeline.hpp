#pragma once

#include "utils/TaskQueue.hpp"

namespace nx
{
  class ChannelPipeline : public TaskQueueRequestSink
  {
  public:
    ChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : m_ctx( ctx ), m_drawPriority( channelId )
    {
      // check the 0th value to see whether the static values haven't been written yet
      if ( m_drawPriorityNames[ 0 ].empty() )
      {
        for ( int32_t i = 0; i < MAX_CHANNELS; ++i )
          m_drawPriorityNames[ i ] = std::to_string( i + 1 );
      }
    }

    ~ChannelPipeline() override = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    virtual nlohmann::json saveChannelPipeline() const = 0;
    virtual void loadChannelPipeline( const nlohmann::json& j ) = 0;

    // this is not hooked up to anything yet
    virtual void processEvent( const sf::Event &event ) const {}
    virtual void update( const sf::Time& deltaTime ) const = 0;
    virtual void drawMenu() = 0;

    sf::RenderTexture * getOutputTexture() const { return m_outputTexture; }
    int32_t getDrawPriority() const { return m_drawPriority; }
    const sf::BlendMode& getChannelBlendMode() const { return m_blendMode; }

  protected:

    void drawChannelPriorityMenu()
    {
      if ( ImGui::BeginCombo( "Draw Priority",
                        m_drawPriorityNames[ m_drawPriority ].c_str() ) )
      {
        for ( int32_t i = 0; i < MAX_MIDI_CHANNELS; ++i )
        {
          if ( ImGui::Selectable( m_drawPriorityNames[ i ].c_str(),
                                  i == m_drawPriority ) )
          {
            m_drawPriority = i;
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
    }

  protected:

    PipelineContext& m_ctx;
    int32_t m_drawPriority;

    bool m_isBypassed { false };

    // this is the final texture handed back to the client
    sf::RenderTexture * m_outputTexture { nullptr };

    sf::BlendMode m_blendMode;

  private:
    inline static std::array< std::string, MAX_CHANNELS > m_drawPriorityNames;
  };
}