#pragma once

#include "models/Interfaces.hpp"

#include "models/data/GlobalInfo_t.hpp"
#include "models/data/Midi_t.hpp"

#include "models/ParticlePipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "shapes/TimedMessage.hpp"

#include <future>

namespace nx
{
  class ChannelPipeline final
  {
  public:

    explicit ChannelPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo ),
        m_particlePipeline( globalInfo ),
        m_shaderPipeline( globalInfo )
    {}

    ~ChannelPipeline() = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const
    {
      nlohmann::json j =
      {
      { "particles", {} },
      { "shaders", {} }
      };

      j[ "particles" ] = m_particlePipeline.saveParticlePipeline();
      j[ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
      return j;
    }

    void loadChannelPipeline( const nlohmann::json& j )
    {
      if ( j.contains( "particles" ) )
        m_particlePipeline.loadParticlePipeline( j.at( "particles" ) );

      if ( j.contains( "shaders" ) )
        m_shaderPipeline.loadShaderPipeline( j.at( "shaders" ) );
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_particlePipeline.processMidiEvent( midiEvent );

      // notify all shaders of an incoming event
      // which can be used for synchronizing effects on midi hits
      m_shaderPipeline.processMidiEvent( midiEvent );
    }

    void processEvent( const sf::Event &event ) const
    {
      m_particlePipeline.processEvent( event );
      m_shaderPipeline.processEvent( event );
    }

    void update( const sf::Time& deltaTime ) const
    {
      m_particlePipeline.update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void draw( sf::RenderWindow& window )
    {
      const auto& particleTexture = m_particlePipeline.draw();
      const auto& shaderTexture = m_shaderPipeline.draw( particleTexture );
      window.draw( sf::Sprite( shaderTexture.getTexture() ), m_blendMode );
    }

    void drawMenu()
    {
      m_particlePipeline.drawMenu();
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
        ImGui::Separator();
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::Separator();
      if ( ImGui::Button( "export" ) )
      {
        const auto json = saveChannelPipeline();
        ImGui::SetClipboardText( json.dump().c_str() );
        m_messageClock.setMessage( "data copied to clipboard." );
      }
      ImGui::SameLine();
      if ( ImGui::Button( "import" ) )
      {
        const auto json = std::string( ImGui::GetClipboardText() );
        if ( !json.empty() )
        {
          const auto importedData =
            nlohmann::json::parse( json.c_str(), nullptr, false );

          if ( !importedData.is_discarded() )
          {
            loadChannelPipeline( importedData );
            m_messageClock.setMessage( "data copied from clipboard." );
          }
          else
            m_messageClock.setMessage( "failed to import: invalid json." );
        }
        else
          m_messageClock.setMessage( "failed to import: empty clipboard." );
      }

      // TODO: add color wheel and slider for cursor time out

      if ( !m_messageClock.hasExpired() )
      {
        ImGui::Separator();
        ImGui::TextUnformatted( m_messageClock.getMessage().data() );
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    ParticlePipeline m_particlePipeline;
    ShaderPipeline m_shaderPipeline;

    TimedMessage m_messageClock;

    bool m_isBypassed { false };
  };
}