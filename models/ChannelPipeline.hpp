#pragma once

#include <memory>

#include "models/Interfaces.hpp"

#include "models/data/GlobalInfo_t.hpp"
#include "models/data/Midi_t.hpp"

#include "models/particle/EmptyParticleLayout.hpp"
#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"

#include "models/modifier/PassthroughParticleModifier.hpp"
#include "models/modifier/ParticleSequentialLineModifier.hpp"
#include "models/modifier/ParticleFullMeshLineModifier.hpp"

#include "models/shader/BlurShader.hpp"
#include "models/shader/KaleidoscopeShader.hpp"
#include "models/shader/GlitchShader.hpp"
#include "models/shader/RippleShader.hpp"
#include "models/shader/StrobeShader.hpp"
#include "models/shader/PulseShader.hpp"

#include "models/ShaderPipeline.hpp"

namespace nx
{
  enum E_LayoutModels
  {
    Empty,
    Spiral,
    Random
  };

  enum E_LineModels
  {
    None,
    Sequential,
    FullMesh
  };

  class ChannelPipeline final
  {
  public:

    explicit ChannelPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo ),
        m_layout( std::make_unique< SpiralParticleLayout >( globalInfo ) ),
        m_modifier( std::make_unique< ParticleSequentialLineModifier >( globalInfo ) ),
        m_shaderPipeline( globalInfo )
    {}

    ~ChannelPipeline() = default;

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_layout->addMidiEvent( midiEvent );

      // notify all shaders of an incoming event
      // which can be used for synchronizing effects on midi hits
      m_shaderPipeline.processMidiEvent( midiEvent );
    }

    void processEvent( const sf::Event &event ) const { m_shaderPipeline.processEvent( event ); }

    void update( const sf::Time& deltaTime ) const
    {
      m_layout->update( deltaTime );
      m_modifier->update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void draw( sf::RenderWindow& window )
    {
      const auto& modifierTexture =
        m_modifier->modifyParticles( m_layout->getParticleOptions(), m_layout->getParticles() );

      // const sf::RenderTexture * currentTexture = &modifierTexture;
      //
      // for ( auto * shader : m_shaders )
      // {
      //   if ( shader->isShaderActive() )
      //     currentTexture = &shader->applyShader( *currentTexture );
      // }

      // window.draw( sf::Sprite( currentTexture->getTexture() ),
      //              m_blendMode );

      const auto& shaderTexture = m_shaderPipeline.draw( modifierTexture );
      window.draw( sf::Sprite( shaderTexture.getTexture() ), m_blendMode );
    }

    void drawMenu()
    {
      m_layout->drawMenu();
      m_modifier->drawMenu();
      m_shaderPipeline.drawMenu();

      ImGui::Separator();
      drawChannelPipelineMenu();
      ImGui::Separator();
    }

  private:

    void drawChannelPipelineMenu()
    {
      if ( ImGui::TreeNode( "Models" ) )
      {

        ////////////////////////////////////////////////////////
        if ( ImGui::TreeNode( "Layouts" ) )
        {
          selectLayoutModel< EmptyParticleLayout >( "Empty" );
          selectLayoutModel< SpiralParticleLayout >( "Spiral" );
          selectLayoutModel< RandomParticleLayout >( "Random" );

          ImGui::TreePop();
          ImGui::Spacing();
        }

        ////////////////////////////////////////////////////////
        if ( ImGui::TreeNode( "Lines" ) )
        {
          selectModifierModel< PassthroughParticleModifier >( "None" );
          selectModifierModel< ParticleSequentialLineModifier >( "Sequential" );
          selectModifierModel< ParticleFullMeshLineModifier >( "Full Mesh" );

          ImGui::TreePop();
          ImGui::Spacing();
        }

        ////////////////////////////////////////////////////////
        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( ImGui::TreeNode( "Global Options" ) )
      {
        ImGui::Separator();
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    template < typename TModel >
    void selectLayoutModel( const std::string& name )
    {
      bool isActive = typeid( *m_layout ) == typeid( TModel );
      if ( ImGui::Checkbox( name.c_str(), &isActive ) )
      {
        if ( isActive )
        {
          LOG_DEBUG( "selected {}", name );
          m_layout.reset( new TModel( m_globalInfo ) );
        }
      }
    }

    template < typename TModel >
    void selectModifierModel( const std::string& name )
    {
      bool isActive = typeid( *m_modifier ) == typeid( TModel );
      if ( ImGui::Checkbox( name.c_str(), &isActive ) )
      {
        if ( isActive )
        {
          LOG_DEBUG( "selected {}", name );
          m_modifier.reset( new TModel( m_globalInfo ) );
        }
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::BlendMode m_blendMode { sf::BlendAdd };

    std::unique_ptr< IParticleLayout > m_layout;
    std::unique_ptr< IParticleModifier > m_modifier;

    ShaderPipeline m_shaderPipeline;

  };
}