#pragma once

#include "models/particle/generator/ParticleGeneratorBase.hpp"

#include "models/particle/particles/StarburstParticle.hpp"

namespace nx
{
  class StarburstParticleGenerator final
    : public ParticleGeneratorBase< StarburstParticleData_t >
  {
  public:

    [[nodiscard]]
    E_ParticleType getType() const override { return E_ParticleType::E_StarburstParticle; }

    void drawMenu() override
    {
      ParticleGeneratorBase::drawMenu();

      auto &data = getData();

      ImGui::SeparatorText("Starburst Options");
      ImGui::SliderInt( "Spikes##1", &data.spikes, 3, 30 );
      ImGui::SliderFloat( "Inner Radius Multiplier", &data.innerRadiusMultiplier, 0.f, 1.f );
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds ) override
    {
      auto * particle = new StarburstParticle( getData(), timeStampInSeconds );
      initialize( particle, midiEvent, timeStampInSeconds );
      return particle;
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds,
                                const float radius ) override
    {
      auto * particle = new StarburstParticle( getData(), timeStampInSeconds, radius );
      initialize( particle, midiEvent, timeStampInSeconds );
      return particle;
    }
  };
}