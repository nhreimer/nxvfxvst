#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "models/data/ParticleLayoutData_t.hpp"

namespace nx
{

  struct LissajousCurveLayoutData_t : public ParticleLayoutData_t
  {
    float phaseAStep { 2.0f };
    float phaseBStep { 3.0f };

    float phaseDelta { 0.5f };
    float phaseSpread { 0.5f };
  };

  /// This layout places each particle along a mathematically beautiful Lissajous curve using parametric sine functions
  /// x = A * sin(a * t + δ)
  /// y = B * sin(b * t)
  /// A = size of window.x
  /// B = size of window.y
  /// a = pitch-based frequency
  /// b = velocity-based frequency
  /// t = time or phase (based on global time)
  /// δ = offset to prevent overlap or to animate drift
  class LissajousCurveLayout final : public ParticleLayoutBase< LissajousCurveLayoutData_t >
  {
  public:

    explicit LissajousCurveLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {}

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