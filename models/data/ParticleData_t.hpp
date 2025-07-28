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

#include "models/ShaderMacros.hpp"
#include "models/easings/TimeEasing.hpp"

namespace nx
{
#define PARTICLE_DATA_PARAMS(X)                                                         \
X( colorEasing,             E_EasingType, E_EasingType::E_Linear, 0, 0, "", false )     \
X( outlineThickness,        float, 0.f  , 0.f, 25.f, "", true )                         \
X( radius,                  float, 10.f , 0.f, 150.f, "", true )                        \
X( pointCount,              int32_t, 30 , 0  , 30, "", true )                           \
X( velocitySizeMultiplier,  float, 1.f  , 0.f, 50.f, "", true )                         \
X( boostVelocity,           float, 0.f  , 0.f, 1.f, "", true )                          \
X( timeoutInSeconds,        float, 0.75f, 0.015f, 5.f, "", true )                       \
X( fillStartColor,  sf::Color, sf::Color(255, 255, 255),    0, 255, "Particle fill color A", false)      \
X( fillEndColor,    sf::Color, sf::Color(255, 255, 255),      0, 255, "Particle fill color B", false)      \
X( outlineStartColor,  sf::Color, sf::Color(255, 255, 255),    0, 255, "Particle fill color A", false)   \
X( outlineEndColor,    sf::Color, sf::Color(255, 255, 255),      0, 255, "Particle fill color B", false)   \

  // holds info ONLY related to the particle
  struct ParticleData_t
  {
    EXPAND_SHADER_PARAMS_FOR_STRUCT(PARTICLE_DATA_PARAMS)
  };

  enum class E_ParticleDataParams
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(PARTICLE_DATA_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_ParticleDataParams::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(PARTICLE_DATA_PARAMS)
  };

}