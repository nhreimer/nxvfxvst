#pragma once

#include "models/particle/particles/RingParticle.hpp"

namespace nx
{
  class RingParticleGenerator final : public ParticleGeneratorBase< RingParticleData_t >
  {
  public:

    explicit RingParticleGenerator( PipelineContext& ctx )
      : ParticleGeneratorBase( ctx, false )
    {
      EXPAND_SHADER_VST_BINDINGS(RING_PARTICLE_PARAMS, ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    E_ParticleType getType() const override { return E_ParticleType::E_RingParticle; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Particle Options" ) )
      {
        EXPAND_SHADER_IMGUI(RING_PARTICLE_PARAMS, getData())

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds ) override
    {
      auto * particle = new RingParticle( getData(), timeStampInSeconds );
      initialize( particle, midiEvent, timeStampInSeconds );
      particle->setOrigin( { 0.f, 0.f } );
      return particle;
    }

    [[nodiscard]]
    IParticle * createParticle( const Midi_t& midiEvent,
                                const float timeStampInSeconds,
                                const float radius ) override
    {
      auto * particle = new RingParticle( getData(), timeStampInSeconds, radius );
      initialize( particle, midiEvent, timeStampInSeconds );
      // reset to 0
      particle->setOrigin( { 0.f, 0.f } );
      return particle;
    }

    [[nodiscard]]
    IParticle * createParticle(const float velocity, const float timeStampInSeconds) override
    {
      auto * particle = new RingParticle( getData(), timeStampInSeconds );
      initialize( particle, velocity, timeStampInSeconds );
      particle->setOrigin( particle->getGlobalBounds().size / 2.f );
      return particle;
    }
  };
}