#pragma once

#include <random>

#include "models/IParticleBehavior.hpp"

namespace nx
{

  class JitterBehavior final : public IParticleBehavior
  {
    struct JitterData_t
    {
      float jitterMultiplier { 0.5 };
    };

  public:
    explicit JitterBehavior( PipelineContext& context )
      : m_ctx( context )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_JitterBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override;

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override;

    void drawMenu() override;

  private:
    sf::Vector2f getJitterPosition( const TimedParticle_t * p );

  private:
      PipelineContext& m_ctx;

      std::mt19937 m_rand;
      JitterData_t m_data;

  };

}