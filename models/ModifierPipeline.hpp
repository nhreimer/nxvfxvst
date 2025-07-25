#pragma once

#include "models/modifier/ParticleFullMeshLineModifier.hpp"

#include "data/PipelineContext.hpp"

namespace nx
{

class ModifierPipeline final
{
public:

  explicit ModifierPipeline( PipelineContext& context )
    : m_ctx( context )
  {}

  void toggleBypass() { m_isBypassed = !m_isBypassed; }

  [[nodiscard]]
  bool isBypassed() const { return m_isBypassed; }

  void clear() { m_modifiers.clear(); }

  void update( const sf::Time& deltaTime ) const;

  void deleteModifier( int position );

  void destroyTextures()
  {
    // ONLY the modifier pipeline has a texture that all the modifiers pass
    // their data to
    m_outputTexture.destroy();
  }

  [[nodiscard]]
  nlohmann::json saveModifierPipeline() const;

  void loadModifierPipeline( const nlohmann::json& j );

  void drawMenu();

  void processMidiEvent( const Midi_t& midiEvent ) const;

  sf::RenderTexture * applyModifiers(
    std::deque< IParticle* >& particles,
    const sf::BlendMode& blendMode );

private:

  void drawParticles( const std::deque< IParticle* >& particles,
                      const sf::BlendMode& blendMode )
  {
    for ( const auto * particle : particles )
      m_outputTexture.draw( *particle, blendMode );
  }

  void drawArtifacts( const std::deque< sf::Drawable* >& artifacts,
                      const sf::BlendMode& blendMode )
  {
    m_artifactCount = artifacts.size();
    for ( const auto * artifact : artifacts )
    {
      m_outputTexture.draw( *artifact, blendMode );
      delete artifact;
    }
  }

  void drawModifierPipelineMenu();

  void drawModifiersAvailable();

  template < typename T >
  IParticleModifier * createModifier()
  {
    auto& modifier =
      m_modifiers.emplace_back< std::unique_ptr< T > >( std::make_unique< T >( m_ctx ) );
    return modifier.get();
  }

  template < typename T >
  IParticleModifier * deserializeModifier( const nlohmann::json& j )
  {
    auto& modifier =
      m_modifiers.emplace_back< std::unique_ptr< T > >( std::make_unique< T >( m_ctx ) );
    modifier->deserialize( j );
    return modifier.get();
  }

private:

  PipelineContext& m_ctx;

  //sf::RenderTexture m_outputTexture;
  LazyTexture m_outputTexture;

  bool m_isBypassed { false };
  sf::BlendMode m_blendMode { sf::BlendAdd };

  std::vector< std::unique_ptr< IParticleModifier > > m_modifiers;

  size_t m_artifactCount { 0 };
};

}