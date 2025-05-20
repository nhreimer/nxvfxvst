#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/data/ParticleLineData_t.hpp"
#include "shapes/CurvedLine.hpp"
#include "helpers/ColorHelper.hpp"

namespace nx
{

  class ParticleFullMeshLineModifier final : public IParticleModifier
  {
#define PARTICLE_LINE_MODIFIER_PARAMS(X)                                                                   \
X(lineThickness,     float,     2.0f,   0.1f,   100.0f,   "Thickness of the curved line",          true)   \
X(swellFactor,       float,     1.5f,   0.0f,   10.0f,   "Swelling multiplier at midpoint",       true)    \
X(easeDownInSeconds, float,     1.0f,   0.01f,  10.0f,   "Ease-out fade duration (seconds)",      true)    \
X(useParticleColors, bool,      true,   0,      1,       "Use original particle colors",          true)    \
X(lineColor,         sf::Color, sf::Color(255,255,255,255), 0, 255, "Primary fallback line color", false)  \
X(otherLineColor,    sf::Color, sf::Color(255,255,255,255), 0, 255, "Alternate/fading line color", false)  \
X(invertColorTime,   bool,      false,  0,      1,       "Colors fade in over time rather than out", true) \
X(curvature,         float,     0.25f,  -NX_PI,  NX_PI,    "Amount of curvature (arc)",             true)  \
X(lineSegments,      int32_t,   20,     1,      200,     "Number of segments in the curve",       true)

    struct FullMeshLineData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(PARTICLE_LINE_MODIFIER_PARAMS)
    };

    enum class E_FullMeshModifierParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(PARTICLE_LINE_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FullMeshModifierParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(PARTICLE_LINE_MODIFIER_PARAMS)
    };

  public:

    explicit ParticleFullMeshLineModifier( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(PARTICLE_LINE_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    E_ModifierType getType() const override { return E_ModifierType::E_FullMeshModifier; }

    void drawMenu() override;

    void update( const sf::Time &deltaTime ) override {}

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< IParticle* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override;

  private:

    PipelineContext& m_ctx;

    bool m_isActive { true };
    FullMeshLineData_t m_data;

  };

}