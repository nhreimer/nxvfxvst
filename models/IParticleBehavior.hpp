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

#include "models/InterfaceTypes.hpp"
#include "models/ISerializable.hpp"
#include "models/IParticle.hpp"
#include "models/data/ParticleData_t.hpp"

namespace nx
{

  struct IParticleBehavior : public ISerializable< E_BehaviorType >
  {
    ~IParticleBehavior() override = default;
    virtual void applyOnSpawn( IParticle * p,
                               const ParticleData_t& particleData ) = 0;

    virtual void applyOnUpdate( IParticle * p,
                                const sf::Time& deltaTime,
                                const ParticleData_t& particleData ) = 0;
    virtual void drawMenu() = 0;
  };

}