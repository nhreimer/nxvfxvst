#pragma once

#include <random>

#include "models/IParticleBehavior.hpp"

namespace nx
{

  class JitterBehavior final : public IParticleBehavior
  {

#define JITTER_BEHAVIOR_PARAMS(X) \
X(jitterMultiplier  , float, 0.5f, 0.f, 5.f, "Amount of jitter", true)

    struct JitterData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(JITTER_BEHAVIOR_PARAMS)
    };

    enum class E_JitterBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(JITTER_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_JitterBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(JITTER_BEHAVIOR_PARAMS)
    };

  public:
    explicit JitterBehavior( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(JITTER_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_JitterBehavior; }

    void applyOnSpawn( IParticle * p,
                       const ParticleData_t& particleData ) override;

    void applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData ) override;

    void drawMenu() override;

  private:
    sf::Vector2f getJitterPosition( const IParticle * p );

  private:
      PipelineContext& m_ctx;

      std::mt19937 m_rand;
      JitterData_t m_data;

  };

}