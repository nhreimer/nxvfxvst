#pragma once

#include "models/particle/EmptyParticleLayout.hpp"
#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"
#include "models/particle/OrbitRingLayout.hpp"

#include "models/modifier/PassthroughParticleModifier.hpp"
#include "models/modifier/ParticleSequentialLineModifier.hpp"
#include "models/modifier/ParticleFullMeshLineModifier.hpp"
#include "models/modifier/TrailEchoModifier.hpp"

namespace nx
{

  class ParticlePipeline final
  {
  public:

    explicit ParticlePipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo ),
        m_layout( std::make_unique< SpiralParticleLayout >( globalInfo ) ),
        m_modifier( std::make_unique< ParticleSequentialLineModifier >( globalInfo ) )
    {}

    ~ParticlePipeline() = default;

    void update( const sf::Time& deltaTime ) const
    {
      m_layout->update( deltaTime );
      m_modifier->update( deltaTime );
    }

    void processEvent( const sf::Event &event ) const {}

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_layout->addMidiEvent( midiEvent );
    }

    void drawMenu()
    {
      if ( ImGui::TreeNode( "Particles" ) )
      {
        ImGui::Text( "Layouts" );
        {
          if ( ImGui::RadioButton( "Empty", m_layout->getType() == E_EmptyLayout ) )
            changeLayout< EmptyParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Spiral", m_layout->getType() == E_SpiralLayout ) )
            changeLayout< SpiralParticleLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Orbit Ring", m_layout->getType() == E_OrbitRingLayout ) )
            changeLayout< OrbitRingLayout >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Random", m_layout->getType() == E_RandomLayout ) )
            changeLayout< RandomParticleLayout >();
        }

        /////////////////////////////////////////////////////

        ImGui::Text( "Modifiers" );
        {
          if ( ImGui::RadioButton( "No Lines", m_modifier->getType() == E_NoModifier ) )
            changeModifier< PassthroughParticleModifier >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Sequential", m_modifier->getType() == E_SequentialModifier ) )
            changeModifier< ParticleSequentialLineModifier >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Mesh", m_modifier->getType() == E_FullMeshModifier ) )
            changeModifier< ParticleFullMeshLineModifier >();

          ImGui::SameLine();
          if ( ImGui::RadioButton( "Trail Echo", m_modifier->getType() == E_TrailEchoModifier ) )
            changeModifier< TrailEchoModifier >();

        }
        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::Separator();

      m_layout->drawMenu();
      m_modifier->drawMenu();
    }

    sf::RenderTexture& draw() const
    {
      return m_modifier->modifyParticles(
        m_layout->getParticleOptions(),
        m_layout->getParticles() );
    }

    nlohmann::json saveParticlePipeline() const
    {
      nlohmann::json j =
      {
        { "layout", {} },
        { "modifier", {} }
      };

      j[ "layout" ] = m_layout->serialize();
      j[ "modifier" ] = m_modifier->serialize();

      return j;
    }

    void loadParticlePipeline( const nlohmann::json& j ) const
    {
      m_layout->deserialize( j.at( "layout" ) );
      m_modifier->deserialize( j.at( "modifier" ) );
    }

  private:

    // save settings between changes to make editing less frustrating
    template < typename T >
    void changeLayout()
    {
      const auto& savedSettings = m_layout->serialize();
      m_layout.reset( new T( m_globalInfo ) );
      m_layout->deserialize( savedSettings );
    }

    template < typename T >
    void changeModifier()
    {
      const auto& savedSettings = m_modifier->serialize();
      m_modifier.reset( new T( m_globalInfo ) );
      m_modifier->deserialize( savedSettings );
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    // these are never null
    std::unique_ptr< IParticleLayout > m_layout;
    std::unique_ptr< IParticleModifier > m_modifier;
  };

}