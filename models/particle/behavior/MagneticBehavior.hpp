#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{

  class MagneticAttractorBehavior final : public IParticleBehavior
  {

#define MAGNET_BEHAVIOR_PARAMS(X)                                                   \
X(isAttracting,     bool        , true,      0,     0, "Attracts or repels", true ) \
X(strength,         float       , 100.f,   0.f, 500.f, "Strength of force" , true ) \
X(useFalloff,       bool        , true,      0,     0, "", true )                   \
X(falloffExponent,  float       , 0.1f,    0.1f,  4.f, "", true )                   \
X(magnetLocation,   sf::Vector2f, sf::Vector2f({0.5f, 0.5f}), 0.f, 0.f, "Location of magnetic source", false )

    struct MagneticData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(MAGNET_BEHAVIOR_PARAMS)
    };

    enum class E_MagneticBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(MAGNET_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_MagneticBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(MAGNET_BEHAVIOR_PARAMS)
    };

  public:
    explicit MagneticAttractorBehavior(PipelineContext& context)
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(MAGNET_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_BehaviorType getType() const override { return E_BehaviorType::E_MagneticBehavior; }

    void applyOnSpawn(IParticle *,
                      const Midi_t&,
                      const ParticleData_t& particleData ) override
    {}

    void applyOnUpdate(IParticle * p,
                       const sf::Time& dt,
                       const ParticleData_t& particleData ) override;

    void drawMenu() override;

  private:

    static float length(const sf::Vector2f& v)
    {
      return std::sqrt(v.x * v.x + v.y * v.y);
    }

  private:
    PipelineContext m_ctx;

    MagneticData_t m_data;
    TimedCursorPosition m_timedCursor;
  };


}