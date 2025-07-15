#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "models/ShaderMacros.hpp"

namespace nx
{

  namespace layout::elliptical
  {
#define ELLIPTICAL_LAYOUT_PARAMS(X)                                                                \
X(radiusX,           float,       300.f,     0.f,     2000.f, "Horizontal radius",         true)   \
X(radiusY,           float,       200.f,     0.f,     2000.f, "Vertical radius",           true)   \
X(arcSpreadDegrees,  float,       360.f,     0.f,     360.f,  "Arc spread in degrees",     true)   \
X(rotationDegrees,   float,       0.f,      -360.f,   360.f,  "Rotation of the ellipse",   true)   \
X(centerOffsetX, float, 0.5f,  0.f, 1.f,   "Horizontal origin (0.0 = left, 1.0 = right)", true)    \
X(centerOffsetY, float, 0.5f,  0.f, 1.f,   "Vertical origin (0.0 = top, 1.0 = bottom)", true)      \
X(sequential,        bool,        false,     0,       1,      "Spawn particles in order",  true)   \
X(slices,            float,       12.f,      1.f,     128.f,  "Number of slices",          true)

    struct EllipticalLayoutData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(ELLIPTICAL_LAYOUT_PARAMS)
    };

    enum class E_BlurParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(ELLIPTICAL_LAYOUT_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_BlurParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(ELLIPTICAL_LAYOUT_PARAMS)
    };
  }

  class EllipticalLayout final
    : public ParticleLayoutBase< layout::elliptical::EllipticalLayoutData_t >
  {

  public:

    explicit EllipticalLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {
      EXPAND_SHADER_VST_BINDINGS(ELLIPTICAL_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_EllipticalLayout; }

    void addMidiEvent(const Midi_t &midiEvent) override;

    void drawMenu() override;

  private:

    void addSequentialParticle( const Midi_t& midiEvent );
    void addParticle( const Midi_t& midiEvent );

  private:

    float m_angleCursor = 0.f; // keeps track of where to place next particle
    TimedCursorPosition m_timedCursor;
  };

}