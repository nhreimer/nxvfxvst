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

namespace nx
{
  ///
  /// used for ImGui to draw a temporary cursor whenever an x, y shift has occurred by the user
  /// so that the user can identify where on the screen something actually is
  class TimedCursorPosition
  {

  public:

    void setPosition( const sf::Vector2f& position,
                      const sf::Vector2f& radius = sf::Vector2f { 15.f, 15.f },
                      int32_t timeoutInSeconds = 5,
                      const ImColor& color = { 127, 127, 127, 255 } )
    {
      m_initialized = true;
      m_position = position;
      m_radius = radius;
      m_color = color;
      m_clock.restart();
    }

    bool hasExpired() const
    {
      return m_initialized && m_clock.getElapsedTime().asSeconds() >= m_timeoutInSeconds;
    }

    void drawPosition() const
    {
      auto * drawList = ImGui::GetBackgroundDrawList();
      //drawList->AddCircle( m_position, m_radius, m_color, 30, 2.f );
      drawList->AddEllipse( m_position, m_radius, m_color );
    }

  private:
    sf::Vector2f m_position;
    sf::Vector2f m_radius;
    sf::Clock m_clock;
    int32_t m_timeoutInSeconds { 5 };
    bool m_initialized { false };
    ImColor m_color;
  };
}