#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

  namespace layout::lsystem
  {
#define LSYSTEM_CURVE_LAYOUT_PARAMS(X)                                                                 \
X(depth,           int,     4,     1,    12,   "Number of recursive steps",             true)          \
X(turnAngle,       float,  30.f, -360.f, 360.f, "Turn angle per branch (degrees)",      true)          \
X(segmentLength,   float,  30.f,  1.f,  500.f, "Length of each forward segment",        true)          \
X(initialAngleDeg, float,   0.f, -360.f, 360.f, "Initial angle of layout (degrees)",    true)          \
X(spreadFactor,    float,   1.f,  0.01f, 10.f,  "Multiplier for how far branches spread", true)        \
X(depthFactor,     float,  0.2f,  0.01f, 1.0f,  "Tightness per recursion level",        true)          \
X(stepsPerNote,    int32_t, 1,     1,   16,     "Curve steps triggered per MIDI note",  true)

    enum class E_LSystemBranchMode : int8_t
    {
      E_Both,
      E_LeftOnly,
      E_RightOnly,
      E_MidiPitch
    };

    struct LSystemCurveLayoutData_t
    {
      bool isActive = true;
      E_LSystemBranchMode branchMode = E_LSystemBranchMode::E_Both;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(LSYSTEM_CURVE_LAYOUT_PARAMS)
    };

    enum class E_LSystemParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(LSYSTEM_CURVE_LAYOUT_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_LSystemParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(LSYSTEM_CURVE_LAYOUT_PARAMS)
    };
  }

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class LSystemCurveLayout final
    : public ParticleLayoutBase< layout::lsystem::LSystemCurveLayoutData_t >
  {

    struct LSystemState_t
    {
      sf::Vector2f position;
      float angleDeg;
      int depth;
      Midi_t midiNote;
    };

  public:
    explicit LSystemCurveLayout(PipelineContext& context)
      : ParticleLayoutBase( context )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_LayoutType getType() const override { return E_LayoutType::E_LSystemCurveLayout; }

    void drawMenu() override;

    void addMidiEvent( const Midi_t &midiEvent ) override;

  private:

    void expandLSystemStep( const Midi_t &midiEvent, const LSystemState_t & state );

  private:

    std::vector< LSystemState_t > m_lsystemStack;
    int32_t m_currentDepth = 1;
  };

} // namespace nx
