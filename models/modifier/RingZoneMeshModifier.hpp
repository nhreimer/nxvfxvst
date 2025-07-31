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

#include "models/IParticleModifier.hpp"
#include "models/ShaderMacros.hpp"
#include "models/data/PipelineContext.hpp"

#include "shapes/CurvedLine.hpp"

namespace nx
{
  /// this works really well with Fractal layouts
  class RingZoneMeshModifier final : public IParticleModifier
  {

#define RING_ZONE_MESH_MODIFIER_PARAMS(X)                                                                       \
X(lineThickness,     float,     2.0f,   0.1f,   100.0f,   "Thickness of the curved line",          true)    \
X(swellFactor,       float,     1.5f,   0.0f,   10.0f,   "Swelling multiplier at midpoint",       true)     \
X(easeDownInSeconds, float,     1.0f,   0.01f,  10.0f,   "Ease-out fade duration (seconds)",      true)     \
X(useParticleColors, bool,      true,   0,      1,       "Use original particle colors",          true)     \
X(lineColor,         sf::Color, sf::Color(255,255,255,255), 0, 255, "Primary fallback line color", false)   \
X(otherLineColor,    sf::Color, sf::Color(255,255,255,255), 0, 255, "Alternate/fading line color", false)   \
X(invertColorTime,   bool,      false,  0,      1,       "Colors fade in over time rather than out", true) \
X(curvature,         float,     0.25f,  -NX_PI,  NX_PI,    "Amount of curvature (arc)",             true)   \
X(lineSegments,      int32_t,   20,     1,      200,     "Number of segments in the curve",       true)     \
X(ringSpacing,   float,   100.0f,   1.0f,   1000.0f,  "Distance between rings",               true)         \
X(drawRings,     bool,    true,     0,      1,        "Toggle drawing of ring meshes",        false)        \
X(drawSpokes,    bool,    true,     0,      1,        "Toggle drawing of radial spokes",      false)
    struct RingZoneMeshData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(RING_ZONE_MESH_MODIFIER_PARAMS)
    };

    enum class E_RingZoneMeshModifierParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(RING_ZONE_MESH_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_RingZoneMeshModifierParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(RING_ZONE_MESH_MODIFIER_PARAMS)
    };

  public:
    explicit RingZoneMeshModifier(PipelineContext& context)
      : m_ctx(context)
    {
      EXPAND_SHADER_VST_BINDINGS(RING_ZONE_MESH_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    E_ModifierType getType() const override { return E_ModifierType::E_RingZoneMeshModifier; }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;
    void drawMenu() override;
    void update(const sf::Time &) override {}

    [[nodiscard]]
    bool isActive() const override { return m_data.isActive; }
    void processMidiEvent(const Midi_t &) override {}

    void modify(const sf::BlendMode& blendMode,
                std::deque< IParticle * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override;

  private:
    static float length(const sf::Vector2f &v) { return std::sqrt(v.x * v.x + v.y * v.y); }

    static float angleFromCenter(const sf::Vector2f &center, const sf::Vector2f &pos)
    {
      const sf::Vector2f d = pos - center;
      return std::atan2(d.y, d.x);
    }

    void setLineColors( CurvedLine * line,
                        const IParticle * pointA,
                        const IParticle * pointB ) const;

  private:
    PipelineContext& m_ctx;
    RingZoneMeshData_t m_data;

  };

} // namespace nx
