/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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