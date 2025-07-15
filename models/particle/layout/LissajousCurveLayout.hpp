#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "models/data/ParticleLayoutData_t.hpp"

namespace nx
{

  namespace layout::lissajous
  {
#define LISSAJOUS_CURVE_LAYOUT_PARAMS(X)                                                              \
X(phaseAStep,    float, 2.0f, 0.1f, 20.0f, "Horizontal oscillation multiplier (A)",  true)            \
X(phaseBStep,    float, 3.0f, 0.1f, 20.0f, "Vertical oscillation multiplier (B)",    true)            \
X(phaseDelta,    float, 0.5f, 0.0f, 2.0f,  "Phase offset between X and Y curves",    true)            \
X(phaseSpread,   float, 0.5f, 0.0f, 5.0f,  "Spread between points along the curve",  true)

    struct LissajousCurveLayoutData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(LISSAJOUS_CURVE_LAYOUT_PARAMS)
    };

    enum class E_LissajousCurveParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(LISSAJOUS_CURVE_LAYOUT_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_LissajousCurveParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(LISSAJOUS_CURVE_LAYOUT_PARAMS)
    };
  }

  /// This layout places each particle along a mathematically beautiful Lissajous curve using parametric sine functions
  /// x = A * sin(a * t + δ)
  /// y = B * sin(b * t)
  /// A = size of window.x
  /// B = size of window.y
  /// a = pitch-based frequency
  /// b = velocity-based frequency
  /// t = time or phase (based on global time)
  /// δ = offset to prevent overlap or to animate drift
  class LissajousCurveLayout final : public ParticleLayoutBase< layout::lissajous::LissajousCurveLayoutData_t >
  {
  public:

    explicit LissajousCurveLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {
      EXPAND_SHADER_VST_BINDINGS(LISSAJOUS_CURVE_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_LissajousCurveLayout; }

    void drawMenu() override;

    void addMidiEvent( const Midi_t &midiEvent ) override;

  private:
    // sf::Vector2f getNextPosition( const Midi_t & midiEvent ) const;

  private:

  };
}