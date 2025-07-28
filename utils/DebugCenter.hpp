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
  struct DebugCenter
  {
    DebugCenter()
    {
      debugCenter.setFillColor( sf::Color::Red );
      const auto& bounds = debugCenter.getGlobalBounds();
      debugCenter.setOrigin( { bounds.size.x / 2, bounds.size.y / 2 } );
    }

    sf::CircleShape debugCenter { 2.f };
  };
}