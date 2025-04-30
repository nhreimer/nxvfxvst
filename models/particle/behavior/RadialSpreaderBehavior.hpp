#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class RadialSpreaderBehavior final : public IParticleBehavior
  {

    struct RadialSpreaderData_t
    {
      float spreadMultiplier = 1.5f;
      float speed = 0.5f;
    };

  public:
    explicit RadialSpreaderBehavior(PipelineContext& context)
      : m_ctx( context )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_RadialSpreaderBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override;
    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override;

    void drawMenu() override;

  private:
    PipelineContext& m_ctx;
    RadialSpreaderData_t m_data;
  };
}