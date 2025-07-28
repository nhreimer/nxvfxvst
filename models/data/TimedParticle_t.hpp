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

#include "../particle/particles/CircleParticle.hpp"

namespace nx
{

  struct TimedParticle_t
  {
    TimedParticle_t() = default;

    TimedParticle_t( const TimedParticle_t& other )
      : shape( other.shape ),
        timeLeft( other.timeLeft ),
        spawnTime( other.spawnTime ), // must keep the time the same or uncoordinated events occur
        initialStartColor( other.initialStartColor ),
        initialEndColor( other.initialEndColor ),
        originalPosition( other.originalPosition )
    {}

    TimedParticle_t& operator=( const TimedParticle_t& other )
    {
      shape = other.shape;
      timeLeft = other.timeLeft;
      spawnTime = other.spawnTime;
      initialStartColor = other.initialStartColor;
      initialEndColor = other.initialEndColor;
      originalPosition = other.originalPosition;
      return *this;
    }

    //Particle shape;
    IParticle * shape;

    int32_t timeLeft { 0 };

    // time in seconds when particle was created
    float spawnTime { 0.f };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialStartColor { sf::Color::White };
    sf::Color initialEndColor { sf::Color::Black };

    // provided by layout
    sf::Vector2f originalPosition;
  };

}