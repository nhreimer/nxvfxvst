#pragma once

#include <memory>

#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"

#include "models/particle/PassthroughParticleModifier.hpp"
#include "models/particle/ParticleSequentialLineModifier.hpp"
#include "models/particle/ParticleFullMeshLineModifier.hpp"

#include "models/shader/BlurShader.hpp"

namespace nx
{
  enum E_LayoutModels
  {
    Spiral,
    Random
  };

  enum E_LineModels
  {
    None,
    Sequential,
    FullMesh
  };

  struct ModelData_t
  {
    E_LayoutModels layoutModel { Spiral };
    E_LineModels lineModel { None };
  };

  class ChannelPipeline final
  {
  public:

    explicit ChannelPipeline( const WindowInfo_t& winfo )
      : m_winfo( winfo ),
        m_layout( new SpiralParticleLayout( winfo ) ),
        m_modifier( new ParticleSequentialLineModifier( winfo ) ),
        m_shader( new BlurShader( winfo ) )
    {}

    ~ChannelPipeline()
    {
      delete m_layout;
      delete m_modifier;
      delete m_shader;
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_layout->addMidiEvent( midiEvent );
    }

    void processEvent( const sf::Event &event ) const
    {}

    void update( const sf::Time& deltaTime ) const
    {
      m_layout->update( deltaTime );
      m_modifier->update( deltaTime );
      m_shader->update( deltaTime );
    }

    void draw( sf::RenderWindow& window ) const
    {
      const auto& modifierTexture =
        m_modifier->modifyParticles( m_layout->getParticleOptions(), m_layout->getParticles() );

      if ( m_shader->isShaderActive() )
      {
        const auto& shaderTexture =
          m_shader->applyShader( modifierTexture );

        window.draw(
          sf::Sprite( shaderTexture.getTexture() ),
          sf::BlendAdd );
      }
      else
      {
        window.draw(
          sf::Sprite( modifierTexture.getTexture() ),
          sf::BlendAdd );
      }
    }

    void drawMenu()
    {
      m_layout->drawMenu();
      m_modifier->drawMenu();
      m_shader->drawMenu();

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
          ptr = new TModel( m_winfo );
        }
      }
    }

  private:

    const WindowInfo_t& m_winfo;

    // std::unique_ptr< IParticleLayout > m_layout;
    // std::unique_ptr< IParticleModifier > m_modifier;
    // std::unique_ptr< IShader > m_shader;

    IParticleLayout * m_layout;
    IParticleModifier * m_modifier;
    IShader * m_shader;

  };
}