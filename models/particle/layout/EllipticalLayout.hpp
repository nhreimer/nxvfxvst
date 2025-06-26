#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

  struct EllipticalLayoutData_t
  {
    float radiusX = 300.f;
    float radiusY = 200.f;
    float arcSpreadDegrees = 360.f; // full ring by default
    float rotationDegrees = 0.f;
    sf::Vector2f centerOffset = {0.f, 0.f};
    bool sequential = false;
    float slices = 12.f;
  };

  class EllipticalLayout final : public ParticleLayoutBase< EllipticalLayoutData_t >
  {
  public:

    explicit EllipticalLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {}

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
  };

}