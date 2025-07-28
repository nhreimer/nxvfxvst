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

#include "models/IParticle.hpp"

namespace nx
{

  class TimedParticleBase : public IParticle
  {
  public:

    void setEnergy(const float energy) override { m_energy = energy; }
    float getEnergy() const override { return m_energy; }

    float getTimeRemainingPercentage() const override
    {
      return m_timeAliveInSeconds / ( m_expirationTimeInSeconds - m_spawnTimeInSeconds );
    }

    bool hasExpired() const override { return getTimeRemainingPercentage() >= 1.0f; }

    float getTimeAliveInSeconds() const override { return m_timeAliveInSeconds; }

    void update( const sf::Time& deltaTime ) override
    {
      m_timeAliveInSeconds += deltaTime.asSeconds();
    }

    float getSpawnTimeInSeconds() const override
    {
      return m_spawnTimeInSeconds;
    }

    void setSpawnTimeInSeconds(const float newSpawnTime) override
    {
      m_spawnTimeInSeconds = newSpawnTime;
    }

    float getExpirationTimeInSeconds() const override
    {
      return m_expirationTimeInSeconds;
    }

    void setExpirationTimeInSeconds(const float expirationTime) override
    {
      m_expirationTimeInSeconds = expirationTime;
    }

  protected:

    float m_spawnTimeInSeconds { 0.f };
    float m_expirationTimeInSeconds { 0.f };
    float m_timeAliveInSeconds { 0.f };
    float m_energy { 0.f };
  };


}