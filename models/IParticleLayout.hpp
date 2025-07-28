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

#include "models/IParticleModifier.hpp"
#include "models/IParticleGenerator.hpp"

namespace nx
{

  class ParticleSequentialLineModifier;
  class ParticleFullMeshLineModifier;
  class PassthroughParticleModifier;

  // a layout has three purposes:
  // 1. facilitate particle generation
  // 2. position the particles
  // 3. apply behaviors
  struct IParticleLayout : ISerializable< E_LayoutType >
  {
    ~IParticleLayout() override = default;

    virtual void addMidiEvent( const Midi_t& midiEvent ) {}
    virtual void processAudioBuffer( const IFFTResult& result ) {}

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual void drawMenu() = 0;

    // [[nodiscard]]
    // virtual const ParticleLayoutData_t& getParticleLayoutData() const = 0;

    [[nodiscard]]
    virtual const ParticleData_t& getParticleData() const = 0;

    /// the implementation of IParticleLayout is the owner of the IParticle
    /// objects. the primary reason why they are not shared_ptr is because shared_ptr
    /// will still leak by inadvertently neglecting to remove objects from the deque.
    /// all particles in the deque get operated on by downstream objects, so if there's a
    /// null dereference then errors will immediately be known and easy to trace.
    [[nodiscard]]
    virtual std::deque< IParticle* >& getParticles() = 0;
  };

}