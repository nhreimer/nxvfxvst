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

#include "TimeEasing.hpp"

namespace nx
{

  // this is good for fast-paced midi events, such as glitch. don't use it on
  // fx like strobe. it is a drop-in replaced for the TimeEasing.
  class CumulativeEasing
  {
  public:

    void deserialize( const nlohmann::json& j )
    {
      m_masterEasing.deserialize( j.at( "easing" ) );
    }

    nlohmann::json serialize() const
    {
      return m_masterEasing.serialize();
    }

    E_EasingType getEasingType() const { return m_masterEasing.getEasingType(); }

    void drawMenu()
    {
      m_masterEasing.drawMenu();
    }

    void trigger()
    {
      auto& burst = m_bursts.emplace_back();
      burst.setData( m_masterEasing.getData() );
      burst.trigger();
      m_lastTriggerInSeconds = m_clock.restart().asSeconds();
    }

    void update( float /*dt*/ )
    {
      std::erase_if( m_bursts,
      []( const TimeEasing& burst )
      {
        return burst.getEasing() <= 0.001f;
      });
    }

    float getEasing() const
    {
      float total = 0.f;
      for ( const auto& b : m_bursts )
        total += b.getEasing();

      // prevent nuclear meltdown
      return std::clamp( total, 0.f, 1.5f );
    }

    float getLastTriggeredInSeconds() const { return m_lastTriggerInSeconds; }

  private:

    float m_lastTriggerInSeconds { 0.f };
    sf::Clock m_clock;

    // the master easing gives a frontend controller for users
    // that sets all easings to the same setting.
    // it would be great to have a way to blend different ones eventually
    TimeEasing m_masterEasing;
    std::vector< TimeEasing > m_bursts;
  };
}