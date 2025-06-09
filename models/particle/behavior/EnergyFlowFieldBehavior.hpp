#pragma once

#include "models/IParticleBehavior.hpp"
#include "models/easings/PercentageEasing.hpp"

namespace nx
{

#define FLOW_BEHAVIOR_PARAMS(X)                                                                      \
X(angleOffset,        float       , 0.f,   -180.f, 180.f, "Angle added to flow field vectors", true) \
X(strength,           float       , 100.f,    0.f, 500.f, "Force applied along flow field", true)    \
X(useFalloff,         bool        , true,       0,     0, "Enable distance-based falloff", true)     \
X(falloffExponent,    float       , 0.75f,    0.1f,  4.f, "Exponent for falloff strength", true)

  struct EnergyFlowFieldBehavior final : public IParticleBehavior
  {
    struct FlowData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(FLOW_BEHAVIOR_PARAMS)
    };

    enum class E_FlowParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(FLOW_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FlowParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(FLOW_BEHAVIOR_PARAMS)
    };

    explicit EnergyFlowFieldBehavior(PipelineContext& ctx)
      : m_ctx(ctx)
    {
      EXPAND_SHADER_VST_BINDINGS(FLOW_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    E_BehaviorType getType() const override { return E_BehaviorType::E_EnergyFlowFieldBehavior; }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(FLOW_BEHAVIOR_PARAMS)
      return j;
    }
    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(FLOW_BEHAVIOR_PARAMS)
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    void applyOnSpawn(IParticle*, const ParticleData_t&) override {}

    void applyOnUpdate(IParticle* p, const sf::Time& dt, const ParticleData_t& particleData) override
    {
      const sf::Vector2f pos = p->getPosition();

      const float energy = p->getEnergy();
      if (energy < 1e-4f) return;

      float angle = energy * 360.f + m_data.angleOffset.first;
      angle = angle * NX_D2R;
      const auto normEnergy = std::clamp( energy, 0.f, 1.f );
      float force = m_data.strength.first * m_easing.getEasing( normEnergy );
      if (m_data.useFalloff.first)
      {
        const float dist = std::max(length(pos - m_ctx.globalInfo.windowHalfSize), 1.f);
        force *= 1.f / std::pow(dist, m_data.falloffExponent.first);
      }

      const auto dir = sf::Vector2f{std::cos(angle), std::sin(angle)};
      const auto offset = dir * force * dt.asSeconds();
      p->move(offset);
    }

    void drawMenu() override
    {
      if (ImGui::TreeNode("Energy Flow Field"))
      {
        EXPAND_SHADER_IMGUI(FLOW_BEHAVIOR_PARAMS, m_data)
        ImGui::Separator();
        m_easing.drawMenu();
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    static float length(const sf::Vector2f& v)
    {
      return std::sqrt(v.x * v.x + v.y * v.y);
    }

    PipelineContext m_ctx;
    FlowData_t m_data;
    PercentageEasing m_easing;
  };

} // namespace nx
