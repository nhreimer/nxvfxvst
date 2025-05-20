#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

  struct GoldenSpiralLayoutData_t : public ParticleLayoutData_t
  {
    int depth = 3; // Total number of particles
    float scaleFactor = 1.1f; // How much each radius increases
    float angleOffset = 0.f; // Rotate the whole spiral
    float baseRadius = 3.f; // radius of circle

    float spiralTightness = 1.f;
    bool useClamp = false;
    bool spiralInward = false;

    // the following two are disabled
    float radiusFalloff = 0.98f;
    bool useRadiusFalloff = false;
  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class GoldenSpiralLayout final : public ParticleLayoutBase< GoldenSpiralLayoutData_t >
  {
  public:
    explicit GoldenSpiralLayout(PipelineContext& context)
        : ParticleLayoutBase(context)
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_GoldenSpiralLayout; }

    void drawMenu() override;

    void addMidiEvent(const Midi_t &midiEvent) override;

  private:

    [[nodiscard]]
    sf::Vector2f getSpiralPosition( const int index,
                                    const int total ) const;

  private:
    static constexpr float GOLDEN_RATIO = 1.61803398875f;
    static constexpr float GOLDEN_ANGLE_DEG = 137.5f;
  };

} // namespace nx
