#pragma once

#include "models/InterfaceTypes.hpp"
#include "models/IParticleBehavior.hpp"

namespace nx
{
  class FreeFallBehavior final : public IParticleBehavior
  {

    struct FreeFallData_t
    {
      float timeDivisor { 2.5f };     // in seconds
    };

  public:
    explicit FreeFallBehavior(PipelineContext& context)
      : m_ctx(context)
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_FreeFallBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override {}

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override;

    void drawMenu() override;

  private:
    PipelineContext& m_ctx;
    FreeFallData_t m_data;
  };
}