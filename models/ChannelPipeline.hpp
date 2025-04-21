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
#include "models/particle/GoldenSpiralLayout.hpp"

#include "models/particle/TestParticleLayout.hpp"

#include <future>

namespace nx
{
  class ChannelPipeline final
  {

  public:

    explicit ChannelPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo ),
        m_particleLayout( std::make_unique< SpiralParticleLayout >( globalInfo ) ),
        m_modifierPipeline( globalInfo ),
        m_shaderPipeline( globalInfo )
    {}

    ~ChannelPipeline() = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const
    {
      nlohmann::json j = {};
      j[ "channel" ][ "particles" ] = m_particleLayout->serialize();
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
          m_particleLayout->deserialize( jchannel.at( "particles" ) );
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
        ImGui::SeparatorText( "Channel Blend" );
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void drawParticleMenu()
    {
      if ( ImGui::TreeNode( "Layouts Available" ) )
      {
        ImGui::Text( "Layouts" );
        {
          if ( ImGui::RadioButton( "Empty", m_particleLayout->getType() == E_LayoutType::E_EmptyLayout ) )
            changeLayout< EmptyParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Random", m_particleLayout->getType() == E_LayoutType::E_RandomLayout ) )
            changeLayout< RandomParticleLayout >();

          ImGui::SeparatorText( "Curved Layouts" );

          if ( ImGui::RadioButton( "Spiral", m_particleLayout->getType() == E_LayoutType::E_SpiralLayout ) )
            changeLayout< SpiralParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Lissajous Curve", m_particleLayout->getType() == E_LayoutType::E_LissajousCurveLayout ) )
            changeLayout< LissajousCurveLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Golden Spiral", m_particleLayout->getType() == E_LayoutType::E_GoldenSpiralLayout ) )
            changeLayout< GoldenSpiralLayout >();

          ImGui::SeparatorText( "Fractal Layouts" );

          if ( ImGui::RadioButton( "Fractal Ring", m_particleLayout->getType() == E_LayoutType::E_FractalRingLayout ) )
            changeLayout< FractalRingLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "L-System Curve", m_particleLayout->getType() == E_LayoutType::E_LSystemCurveLayout ) )
            changeLayout< LSystemCurveLayout >();

#ifdef DEBUG
          if ( ImGui::RadioButton( "Test", m_particleLayout->getType() == E_LayoutType::E_TestLayout ) )
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
      m_tempSettings[ SerialHelper::serializeEnum( m_particleLayout->getType() ) ] = m_particleLayout->serialize();
      m_particleLayout.reset( new T( m_globalInfo ) );

      const auto newLayoutName = SerialHelper::serializeEnum( m_particleLayout->getType() );
      if ( m_tempSettings.contains( newLayoutName ) )
        m_particleLayout->deserialize( m_tempSettings[ newLayoutName ] );
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    std::atomic< bool > m_isReady { false };

    nlohmann::json m_tempSettings;

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    //ParticlePipeline m_particlePipeline;
    std::unique_ptr< IParticleLayout > m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    bool m_isBypassed { false };
  };
}