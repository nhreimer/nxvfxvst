#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

  enum class E_LSystemBranchMode : int8_t
  {
    E_Both,
    E_LeftOnly,
    E_RightOnly,
    E_MidiPitch
  };

  struct LSystemCurveLayoutData_t : public ParticleLayoutData_t
  {
    int depth = 4; // number of recursive steps
    float turnAngle = 30.f; // degrees turned on L/R
    float segmentLength = 30.f; // length of each forward step
    float initialAngleDeg = 0.f;
    float spreadFactor = 1.f;   // how far to spread out branches
    float depthFactor = 0.2f;   // how tight the particles are to each other
    int32_t stepsPerNote = 1; // 2, 3, etc.
    E_LSystemBranchMode m_branchMode = E_LSystemBranchMode::E_Both;
  };

  struct LSystemState_t
  {
    sf::Vector2f position;
    float angleDeg;
    int depth;
    Midi_t midiNote;
  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class LSystemCurveLayout final : public ParticleLayoutBase< LSystemCurveLayoutData_t >
  {

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
    int m_currentDepth = 1;
  };

} // namespace nx
