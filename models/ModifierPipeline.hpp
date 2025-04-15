#pragma once

#include "models/modifier/ParticleSequentialLineModifier.hpp"
#include "models/modifier/ParticleFullMeshLineModifier.hpp"
#include "models/modifier/PerlinDeformerModifier.hpp"

#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#include "models/modifier/TestModifier.hpp"
#endif

namespace nx
{

class ModifierPipeline final
{
public:

  explicit ModifierPipeline( const GlobalInfo_t& globalInfo )
    : m_globalInfo( globalInfo )
  {}

  void update( const sf::Time& deltaTime ) const
  {
    for ( auto& modifier : m_modifiers ) modifier->update( deltaTime );
  }

  void toggleBypass() { m_isBypassed = !m_isBypassed; }
  bool isBypassed() const { return m_isBypassed; }

  void clear() { m_modifiers.clear(); }

  void deleteModifier( const int position )
  {
    assert( position >= 0 && position < m_modifiers.size() );
    m_modifiers.erase( m_modifiers.begin() + position );
  }

  nlohmann::json saveModifierPipeline() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( const auto& modifier : m_modifiers )
      j.push_back( modifier->serialize() );

    return j;
  }

  void loadModifierPipeline( const nlohmann::json& j )
  {
    m_modifiers.clear();
    for ( const auto& modifierData : j )
    {
      const auto type =
        SerialHelper::convertStringToModifierType( modifierData.value("type", "" ) );
      switch ( type )
      {
        case E_ModifierType::E_SequentialModifier:
          deserializeModifier< ParticleSequentialLineModifier >( modifierData );
          break;

        case E_ModifierType::E_FullMeshModifier:
          deserializeModifier< ParticleFullMeshLineModifier >( modifierData );
          break;

        case E_ModifierType::E_PerlinDeformerModifier:
          deserializeModifier< PerlinDeformerModifier >( modifierData );
          break;

        default:
          LOG_ERROR( "unable to deserialize modifier type" );
          break;
      }
    }
  }

  void drawMenu()
  {
    drawModifiersAvailable();
    drawModifierPipelineMenu();
  }

  void processMidiEvent( const Midi_t& midiEvent ) const
  {
    for ( auto& modifier : m_modifiers )
      modifier->processMidiEvent( midiEvent );
  }

  sf::RenderTexture& applyModifiers(
    const ParticleLayoutData_t& particleLayoutData,
    std::deque< TimedParticle_t* >& particles )
  {
    if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
    {
      if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
      {
        LOG_ERROR( "Failed to resize modifier pipeline texture" );
      }
    }

    std::deque< sf::Drawable* > newArtifacts;

    for ( const auto& modifier : m_modifiers )
    {
      if ( modifier->isActive() )
        modifier->modify( particleLayoutData, particles, newArtifacts );
    }

    m_outputTexture.clear( sf::Color::Transparent );

    drawArtifacts( newArtifacts, m_blendMode );
    drawParticles( particles, particleLayoutData.blendMode );

    m_outputTexture.display();
    return m_outputTexture;
  }

private:

  void drawParticles( const std::deque< TimedParticle_t* >& particles,
                      const sf::BlendMode& blendMode )
  {
    for ( const auto * particle : particles )
      m_outputTexture.draw( particle->shape, blendMode );
  }

  void drawArtifacts( const std::deque< sf::Drawable* >& artifacts,
                      const sf::BlendMode& blendMode )
  {
    for ( const auto * artifact : artifacts )
    {
      m_outputTexture.draw( *artifact, blendMode );
      delete artifact;
    }
  }

  void drawModifierPipelineMenu()
  {
    ImGui::Separator();
    ImGui::Text( "Modifiers: %d", m_modifiers.size() );

    int deletePos = -1;
    int swapA = -1;
    int swapB = -1;

    if ( ImGui::TreeNode( "Active Modifiers" ) )
    {
      for ( int i = 0; i < m_modifiers.size(); ++i )
      {
        ImGui::PushID( i );

        if ( i > 0 )
          ImGui::Separator();

        if ( ImGui::Button( "x" ) )
          deletePos = i;
        else
        {
          ImGui::SameLine();
          m_modifiers[ i ]->drawMenu();

          if ( ImGui::Button( "u" ) )
          {
            swapA = i;
            swapB = i - 1;
          }

          ImGui::SameLine();

          if ( ImGui::Button( "d" ) )
          {
            swapA = i;
            swapB = i + 1;
          }
        }
        ImGui::PopID();
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( deletePos > -1 )
      m_modifiers.erase( m_modifiers.begin() + deletePos );
    else if ( swapA > -1 && swapB > -1 && swapA < m_modifiers.size() && swapB < m_modifiers.size() )
      std::swap( m_modifiers[ swapA ], m_modifiers[ swapB ] );
  }

  void drawModifiersAvailable()
  {
    if ( ImGui::TreeNode( "Mods Available" ) )
    {
      MenuHelper::drawBlendOptions( m_blendMode );
      ImGui::NewLine();

      if ( ImGui::Button( "Seq Line##1" ) )
        createModifier< ParticleSequentialLineModifier >();

      ImGui::SameLine();
      if ( ImGui::Button( "Mesh Line##1" ) )
        createModifier< ParticleFullMeshLineModifier >();

      ImGui::SameLine();
      if ( ImGui::Button( "Perlin Deformer##3" ) )
        createModifier< PerlinDeformerModifier >();

#ifdef DEBUG
      if ( ImGui::Button( "Test##3" ) )
        createModifier< TestModifier >();
#endif

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  template < typename T >
  IParticleModifier * createModifier()
  {
    auto& modifier = m_modifiers.emplace_back< std::unique_ptr< T > >(
      std::make_unique< T >( m_globalInfo ) );
    return modifier.get();
  }

  template < typename T >
  IParticleModifier * deserializeModifier( const nlohmann::json& j )
  {
    auto& modifier = m_modifiers.emplace_back< std::unique_ptr< T > >(
      std::make_unique< T >( m_globalInfo ) );
    modifier->deserialize( j );
    return modifier.get();
  }

private:

  const GlobalInfo_t& m_globalInfo;

  sf::RenderTexture m_outputTexture;

  bool m_isBypassed { false };
  sf::BlendMode m_blendMode { sf::BlendAdd };

  std::vector< std::unique_ptr< IParticleModifier > > m_modifiers;
};

}