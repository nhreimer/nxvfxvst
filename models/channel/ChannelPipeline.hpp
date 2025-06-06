#pragma once

#include "utils/TaskQueue.hpp"

#include "models/ParticleLayoutManager.hpp"
#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

namespace nx
{
  class ChannelPipeline : public TaskQueueRequestSink
  {
  public:
    ChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : m_ctx( ctx ),
        m_drawPriority( channelId ),
        m_particleLayout( ctx ),
        m_modifierPipeline( ctx ),
        m_shaderPipeline( ctx, *this )
    {
      // check the 0th value to see whether the static values haven't been written yet
      if ( m_drawPriorityNames[ 0 ].empty() )
      {
        for ( int32_t i = 0; i < MAX_CHANNELS; ++i )
          m_drawPriorityNames[ i ] = std::to_string( i + 1 );
      }
    }

    ~ChannelPipeline() override = default;

    virtual nlohmann::json saveChannelPipeline() const
    {
      nlohmann::json j = {};
      j[ "channel" ][ "particles" ] = m_particleLayout.serialize();
      j[ "channel" ][ "modifiers" ] = m_modifierPipeline.saveModifierPipeline();
      j[ "channel" ][ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
      return j;
    }

    virtual void loadChannelPipeline( const nlohmann::json& j )
    {
      if ( j.contains( "channel" ) )
      {
        const auto& jchannel = j[ "channel" ];
        if ( jchannel.contains( "particles" ) )
          m_particleLayout.deserialize( jchannel.at( "particles" ) );
        else
        {
          // a channel particle should always be available
          LOG_WARN( "Deserializer: Channel particle layout not found" );
        }

        if ( jchannel.contains( "modifiers" ) )
          m_modifierPipeline.loadModifierPipeline( jchannel.at( "modifiers" ) );
        else
        {
          // optional whether any exist
          LOG_DEBUG( "Deserializer: No channel modifiers found" );
        }

        if ( jchannel.contains( "shaders" ) )
          m_shaderPipeline.loadShaderPipeline( jchannel.at( "shaders" ) );
        else
        {
          // optional whether any exist
          LOG_DEBUG( "Deserializer: No channel shaders found" );
        }
      }
      else
      {
        LOG_WARN( "Deserializer: No channel data found" );
      }
    }

    void requestRenderUpdate() override
    {
      // this comes in on the main thread,
      // but we need to move it to the render thread
      request( [ this ]
      {
        const auto * modifierTexture = m_modifierPipeline.applyModifiers(
          m_particleLayout.getParticleOptions(),
          m_particleLayout.getParticles() );

        m_outputTexture = m_shaderPipeline.draw( modifierTexture );
      } );
    }

    void requestShutdown() override
    {
      request( [ this ]
      {
        // this asks all other pipelines to shut down
        m_modifierPipeline.destroyTextures();
        m_shaderPipeline.destroyTextures();
      } );
    }

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    // this is not hooked up to anything yet
    virtual void processEvent( const sf::Event &event ) const {}
    virtual void update( const sf::Time& deltaTime ) const = 0;
    virtual void drawMenu() = 0;

    sf::RenderTexture * getOutputTexture() const { return m_outputTexture; }
    int32_t getDrawPriority() const { return m_drawPriority; }
    const sf::BlendMode& getChannelBlendMode() const { return m_blendMode; }

  protected:

    virtual void drawChannelPriorityMenu()
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

    virtual void drawChannelPipelineMenu()
    {
      if ( ImGui::TreeNode( "Channel Options" ) )
      {
        ImGui::Checkbox( "Mute", &m_isBypassed );

        ImGui::SeparatorText( "Channel Blend" );
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  protected:

    PipelineContext& m_ctx;
    int32_t m_drawPriority;

    ParticleLayoutManager m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    bool m_isBypassed { false };

    // this is the final texture handed back to the client
    sf::RenderTexture * m_outputTexture { nullptr };

    sf::BlendMode m_blendMode;

  private:
    inline static std::array< std::string, MAX_CHANNELS > m_drawPriorityNames;
  };
}