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
  struct ParticleLineData_t
  {
    // sf::Color startColor { 255, 255, 255 };
    // sf::Color endColor { 0, 0, 0 };
    bool isActive{ true };
    float lineThickness{ 2.f };
    float swellFactor { 1.5f  };
    float easeDownInSeconds { 1.f };

    bool useParticleColors { true };
    sf::Color lineColor = sf::Color(255, 255, 255, 255);
    sf::Color otherLineColor = sf::Color(255, 255, 255, 255);

    float curvature { 0.25f };
    int32_t lineSegments { 20 };
  };

}