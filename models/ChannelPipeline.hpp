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

#include "models/ShaderManager.hpp"

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
        m_layout( new SpiralParticleLayout( globalInfo ) ),
        m_modifier( new ParticleSequentialLineModifier( globalInfo ) ),
        m_shaders( { new GlitchShader( globalInfo ),
                     new KaleidoscopeShader( globalInfo ),
                     new RippleShader( globalInfo ),
                     new PulseShader( globalInfo ),
                     new StrobeShader( globalInfo ),
                     new BlurShader( globalInfo ) } )
    {}

    ~ChannelPipeline()
    {
      delete m_layout;
      delete m_modifier;
      for ( const auto * shader : m_shaders )
        delete shader;
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_layout->addMidiEvent( midiEvent );

      // notify all shaders of an incoming event
      // which can be used for synchronizing effects on midi hits
      for ( auto * shader : m_shaders )
      {
        if ( shader )
          shader->trigger( midiEvent );
      }
    }

    void processEvent( const sf::Event &event ) const
    {}

    void update( const sf::Time& deltaTime ) const
    {
      m_layout->update( deltaTime );
      m_modifier->update( deltaTime );
      for ( auto * shader : m_shaders )
        shader->update( deltaTime );
    }

    void draw( sf::RenderWindow& window ) const
    {
      const auto& modifierTexture =
        m_modifier->modifyParticles( m_layout->getParticleOptions(), m_layout->getParticles() );

      const sf::RenderTexture * currentTexture = &modifierTexture;

      for ( auto * shader : m_shaders )
      {
        if ( shader->isShaderActive() )
          currentTexture = &shader->applyShader( *currentTexture );
      }

      window.draw( sf::Sprite( currentTexture->getTexture() ),
                   m_blendMode );
    }

    void drawMenu()
    {
      m_layout->drawMenu();
      m_modifier->drawMenu();
      for ( auto * shader : m_shaders )
        shader->drawMenu();

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
          selectModel( static_cast< EmptyParticleLayout * >( m_layout ), "Empty" );
          selectModel( static_cast< SpiralParticleLayout * >( m_layout ), "Spiral" );
          selectModel( static_cast< RandomParticleLayout * >( m_layout ), "Random" );

          ImGui::TreePop();
          ImGui::Spacing();
        }

        ////////////////////////////////////////////////////////
        if ( ImGui::TreeNode( "Lines" ) )
        {
          selectModel( static_cast< PassthroughParticleModifier * >( m_modifier ), "None" );
          selectModel( static_cast< ParticleSequentialLineModifier * >( m_modifier ), "Sequential" );
          selectModel( static_cast< ParticleFullMeshLineModifier * >( m_modifier ), "Full Mesh" );

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
    void selectModel( TModel * ptr,
                      const std::string& name )
    {
      bool isActive = typeid( *ptr ) == typeid( TModel );
      if ( ImGui::Checkbox( name.c_str(), &isActive ) )
      {
        if ( isActive )
        {
          LOG_DEBUG( "selected {}", name );
          delete ptr;
          ptr = new TModel( m_globalInfo );
        }
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::BlendMode m_blendMode { sf::BlendAdd };

    // std::unique_ptr< IParticleLayout > m_layout;
    // std::unique_ptr< IParticleModifier > m_modifier;
    // std::unique_ptr< IShader > m_shader;

    IParticleLayout * m_layout { nullptr };
    IParticleModifier * m_modifier { nullptr };
    std::vector< IShader * > m_shaders;

  };
}