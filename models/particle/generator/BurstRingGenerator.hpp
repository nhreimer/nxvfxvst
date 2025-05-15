#pragma once

#include "models/particle/particles/BurstRingParticle.hpp"

namespace nx
{
  class BurstRingGenerator final : public ParticleGeneratorBase< BurstRingParticleData_t >
  {
  public:

    [[nodiscard]]
    E_ParticleType getType() const override { return E_ParticleType::E_BurstRingParticle; }

    void drawMenu() override
    {
      ParticleGeneratorBase::drawMenu();

      auto &data = getData();

      ImGui::SeparatorText("Burst Ring Options");
      ImGui::SliderInt( "Spikes##1", &data.spokes, 4, 64 );
      ImGui::SliderFloat( "Spike Length Multiplier##1", &data.spokeLengthMultiplier, 0.f, 10.f );
      ImGui::SliderFloat( "Spike Thickness##1", &data.spokeThickness, 0.f, 32.f );
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds ) override
    {
      auto * particle = new BurstRingParticle( getData(), timeStampInSeconds );
      initialize( particle, midiEvent, timeStampInSeconds );
      return particle;
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds,
                                const float radius ) override
    {
      auto * particle = new BurstRingParticle( getData(), timeStampInSeconds, radius );
      initialize( particle, midiEvent, timeStampInSeconds );
      return particle;
    }
  };
}