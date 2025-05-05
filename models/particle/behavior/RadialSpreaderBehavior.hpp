#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class RadialSpreaderBehavior final : public IParticleBehavior
  {

#define RADIAL_SPREADER_BEHAVIOR_PARAMS(X)                                       \
X(spreadMultiplier    , float , 1.5f  ,   0.f ,   5.f, "Amount of spread", true) \
X(speed               , float , 0.5f  ,   0.f ,   5.f, "Speed of spread", true)

    struct RadialSpreaderData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(RADIAL_SPREADER_BEHAVIOR_PARAMS)
    };

    enum class E_RadialSpreadBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(RADIAL_SPREADER_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_RadialSpreadBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(RADIAL_SPREADER_BEHAVIOR_PARAMS)
    };


  public:
    explicit RadialSpreaderBehavior(PipelineContext& context)
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(RADIAL_SPREADER_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

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