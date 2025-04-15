#pragma once

#include "models/Interfaces.hpp"

#include "models/data/GlobalInfo_t.hpp"
#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "models/particle/EmptyParticleLayout.hpp"
#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"

#include "models/particle/LissajousCurveLayout.hpp"
#include "models/particle/FractalRingLayout.hpp"
#include "models/particle/LSystemCurveLayout.hpp"

#include "models/particle/TestParticleLayout.hpp"

#include "shapes/TimedMessage.hpp"

#include <future>

namespace nx
{
  class ChannelPipeline final
  {
  public:

    explicit ChannelPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo(globalInfo),
        m_particleLayout(std::make_unique< SpiralParticleLayout >(globalInfo)),
        m_modifierPipeline(globalInfo),
        m_shaderPipeline(globalInfo)
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

      j[ "particles" ] = m_particleLayout->serialize();
      j[ "modifiers" ] = m_modifierPipeline.saveModifierPipeline();
      j[ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
      return j;
    }

    void loadChannelPipeline( const nlohmann::json& j )
    {
      if ( j.contains( "particles" ) )
        m_particleLayout->deserialize( j.at( "particles" ) );

      if ( j.contains( "modifiers" ) )
        m_modifierPipeline.loadModifierPipeline( j.at( "modifiers" ) );

      if ( j.contains( "shaders" ) )
        m_shaderPipeline.loadShaderPipeline( j.at( "shaders" ) );
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_particleLayout->addMidiEvent( midiEvent );

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
      m_particleLayout->update( deltaTime );
      m_modifierPipeline.update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void draw( sf::RenderWindow& window )
    {
      const auto& modifierTexture = m_modifierPipeline.applyModifiers(
        m_particleLayout->getParticleOptions(),
        m_particleLayout->getParticles() );

      const auto& shaderTexture = m_shaderPipeline.draw( modifierTexture );
      window.draw( sf::Sprite( shaderTexture.getTexture() ), m_blendMode );
    }

    void drawMenu()
    {
      drawParticleMenu();

      ImGui::Separator();
      m_particleLayout->drawMenu();

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

    void drawParticleMenu()
    {
      if ( ImGui::TreeNode( "Particles" ) )
      {
        ImGui::Text( "Layouts" );
        {
          if ( ImGui::RadioButton( "Empty", m_particleLayout->getType() == E_EmptyLayout ) )
            changeLayout< EmptyParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Random", m_particleLayout->getType() == E_RandomLayout ) )
            changeLayout< RandomParticleLayout >();


          if ( ImGui::RadioButton( "Spiral", m_particleLayout->getType() == E_SpiralLayout ) )
            changeLayout< SpiralParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Lissajous Curve", m_particleLayout->getType() == E_LissajousCurveLayout ) )
            changeLayout< LissajousCurveLayout >();


          if ( ImGui::RadioButton( "Fractal Ring", m_particleLayout->getType() == E_FractalRingLayout ) )
            changeLayout< FractalRingLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "L-System Curve", m_particleLayout->getType() == E_LSystemCurveLayout ) )
            changeLayout< LSystemCurveLayout >();

#ifdef DEBUG
          if ( ImGui::RadioButton( "Test", m_particleLayout->getType() == E_TestLayout ) )
            changeLayout< TestParticleLayout >();
#endif
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    // save settings between changes to make editing less frustrating
    template < typename T >
    void changeLayout()
    {
      const auto& savedSettings = m_particleLayout->serialize();
      m_particleLayout.reset( new T( m_globalInfo ) );
      m_particleLayout->deserialize( savedSettings );
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    //ParticlePipeline m_particlePipeline;
    std::unique_ptr< IParticleLayout > m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    TimedMessage m_messageClock;

    bool m_isBypassed { false };
  };
}