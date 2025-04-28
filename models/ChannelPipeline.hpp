#pragma once

#include "models/Interfaces.hpp"

#include "models/data/GlobalInfo_t.hpp"
#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "data/PipelineContext.hpp"
#include "models/ParticleLayoutManager.hpp"

#include <future>


namespace nx
{
  class ChannelPipeline final
  {

  public:

    explicit ChannelPipeline( PipelineContext& context )
      : m_ctx( context ),
        m_particleLayout( context ),
        m_modifierPipeline( context ),
        m_shaderPipeline( context )
    {}

    ~ChannelPipeline() = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const
    {
      nlohmann::json j = {};
      j[ "channel" ][ "particles" ] = m_particleLayout.serialize();
      j[ "channel" ][ "modifiers" ] = m_modifierPipeline.saveModifierPipeline();
      j[ "channel" ][ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
      return j;
    }

    void loadChannelPipeline( const nlohmann::json& j )
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

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_particleLayout.processMidiEvent( midiEvent );

      m_modifierPipeline.processMidiEvent( midiEvent );

      // notify all shaders of an incoming event
      // which can be used for synchronizing effects on midi hits
      m_shaderPipeline.processMidiEvent( midiEvent );
    }

    void processEvent( const sf::Event &event ) const
    {
      // TODO: add? maybe?
    }

    void update( const sf::Time& deltaTime ) const
    {
      m_particleLayout.update( deltaTime );
      m_modifierPipeline.update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void draw( sf::RenderWindow& window )
    {
      const auto& modifierTexture = m_modifierPipeline.applyModifiers(
        m_particleLayout.getParticleOptions(),
        m_particleLayout.getParticles() );

      const auto& shaderTexture = m_shaderPipeline.draw( modifierTexture );
      window.draw( sf::Sprite( shaderTexture.getTexture() ), m_blendMode );
    }

    void drawMenu()
    {
      m_particleLayout.drawMenu();

      ImGui::Separator();
      m_modifierPipeline.drawMenu();

      ImGui::Separator();
      m_shaderPipeline.drawMenu();

      ImGui::Separator();
      drawChannelPipelineMenu();
      ImGui::Separator();
    }

  private:

    void drawChannelPipelineMenu()
    {
      if ( ImGui::TreeNode( "Global Options" ) )
      {
        ImGui::SeparatorText( "Channel Blend" );
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    // const GlobalInfo_t& m_globalInfo;
    PipelineContext& m_ctx;

    std::atomic< bool > m_isReady { false };

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    ParticleLayoutManager m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    bool m_isBypassed { false };
  };
}