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

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

  namespace layout::goldenspiral
  {
#define GOLDEN_SPIRAL_LAYOUT_PARAMS(X)                                                                       \
X(depth,             int,     3,      1,   512,    "Total number of particles",              true)           \
X(scaleFactor,       float,   1.1f,   0.01f, 5.0f, "Radius increase multiplier",             true)           \
X(angleOffset,       float,   0.0f,  -360.f, 360.f,"Rotation of entire spiral",             true)            \
X(baseRadius,        float,   3.0f,   0.1f, 100.f, "Initial particle radius",               true)            \
X(spiralTightness,   float,   1.0f,   0.01f, 10.0f,"Spacing of spiral curve",               true)            \
X(useClamp,          bool,    false,  0,    1,     "Clamp max radius based on screen",      false)           \
X(spiralInward,      bool,    false,  0,    1,     "Spiral inward instead of outward",      false)           \
X(radiusFalloff,     float,   0.98f,  0.0f, 1.0f,  "Unused: radius falloff per step",       false)           \
X(useRadiusFalloff,  bool,    false,  0,    1,     "Unused: enable radius falloff",         false)

    struct GoldenSpiralLayoutData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(GOLDEN_SPIRAL_LAYOUT_PARAMS)
    };

    enum class E_GoldenSpiralParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(GOLDEN_SPIRAL_LAYOUT_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_GoldenSpiralParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(GOLDEN_SPIRAL_LAYOUT_PARAMS)
    };

  }

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class GoldenSpiralLayout final : public ParticleLayoutBase< layout::goldenspiral::GoldenSpiralLayoutData_t >
  {
  public:
    explicit GoldenSpiralLayout(PipelineContext& context)
        : ParticleLayoutBase(context)
    {
      EXPAND_SHADER_VST_BINDINGS(GOLDEN_SPIRAL_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_GoldenSpiralLayout; }

    void drawMenu() override;

    void addMidiEvent(const Midi_t &midiEvent) override;

  private:

    [[nodiscard]]
    sf::Vector2f getSpiralPosition( const int index,
                                    const int total ) const;

  private:

    TimedCursorPosition m_timedCursorPosition;

    static constexpr float GOLDEN_RATIO = 1.61803398875f;
    static constexpr float GOLDEN_ANGLE_DEG = 137.5f;

  };

} // namespace nx
