#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class ParticleBehaviorPipeline final
  {
  public:

    explicit ParticleBehaviorPipeline( PipelineContext& context )
      : m_ctx( context )
    {}

    nlohmann::json savePipeline() const;
    void loadPipeline( const nlohmann::json& j );

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) const;
    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) const;

    void drawMenu();

  private:

    void drawBehaviorPipelineMenu();
    void drawBehaviorsAvailable();

    template < typename T >
    IParticleBehavior * createBehavior()
    {
      auto& behavior = m_particleBehaviors.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      return behavior.get();
    }


    template < typename T >
    IParticleBehavior * deserializeBehavior( const nlohmann::json& j )
    {
      auto& behavior = m_particleBehaviors.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      behavior->deserialize( j );
      return behavior.get();
    }

  private:

    PipelineContext& m_ctx;
    std::vector< std::unique_ptr< IParticleBehavior > > m_particleBehaviors;
  };

}