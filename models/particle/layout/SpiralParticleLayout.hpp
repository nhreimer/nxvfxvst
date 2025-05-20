#pragma once

#include "ParticleLayoutBase.hpp"

namespace nx
{

  class SpiralParticleLayout final : public ParticleLayoutBase< ParticleLayoutData_t >
  {
  public:

    explicit SpiralParticleLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {}

    ~SpiralParticleLayout() override = default;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_SpiralLayout; }

    [[nodiscard]]
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json &j) override;

    void addMidiEvent(const Midi_t &midiEvent) override;

    void drawMenu() override;

  protected:

    [[nodiscard]]
    sf::Vector2f getNextPosition( const Midi_t& midiNote ) const;

  private:

  };

}