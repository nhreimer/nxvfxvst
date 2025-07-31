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

#include "models/data/ParticleData_t.hpp"
#include "models/data/Midi_t.hpp"

namespace nx
{
  struct IFFTResult;

  // returns the created particle and the bin it came from
  using ParticleCreatedCallback = std::function< void( IParticle *, int32_t ) >;

  struct IParticleGenerator : public ISerializable< E_ParticleType >
  {
    [[nodiscard]]
    virtual const ParticleData_t& getData() const = 0;
    virtual void drawMenu() = 0;

    /// used for creating one not based on a midi event one that just
    /// uses velocity or magnitude or amplitude that goes towards the
    /// initial brightness of the particle. in midi events, that's based on the
    /// velocity. for audio data, it can be based on magnitude or energy or whatever you want
    [[nodiscard]]
    virtual IParticle * createParticle( float velocity, float timeStampInSeconds ) = 0;

    [[nodiscard]]
    virtual IParticle * createParticle( const Midi_t& midiEvent,
                                        float timeStampInSeconds ) = 0;

    // same as the one above but allows us to set the radius
    // in a layout needs to make any adjustment
    [[nodiscard]]
    virtual IParticle * createParticle( const Midi_t& midiEvent,
                                        float timeStampInSeconds,
                                        float radius ) = 0;
  };
}