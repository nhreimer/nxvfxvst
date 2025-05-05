#pragma once

#include "models/InterfaceTypes.hpp"
#include "models/IParticleBehavior.hpp"

namespace nx
{
  class FreeFallBehavior final : public IParticleBehavior
  {

#define FREE_FALL_BEHAVIOR_PARAMS(X)                                            \
X(timeDivisor,  float,  2.5f,   0.f, 50.f, "Time in seconds of falling", true )

    struct FreeFallData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(FREE_FALL_BEHAVIOR_PARAMS)
    };

    enum class E_FreeFallBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(FREE_FALL_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FreeFallBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(FREE_FALL_BEHAVIOR_PARAMS)
    };

  public:
    explicit FreeFallBehavior(PipelineContext& context)
      : m_ctx(context)
    {
      EXPAND_SHADER_VST_BINDINGS(FREE_FALL_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

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