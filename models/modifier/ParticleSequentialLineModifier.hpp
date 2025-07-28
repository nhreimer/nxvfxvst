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

#include "shapes/CurvedLine.hpp"
#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class ParticleSequentialLineModifier final : public IParticleModifier
  {
#define PARTICLE_LINE_MODIFIER_PARAMS(X)                                                                   \
X(lineThickness,     float,     2.0f,   0.1f,   100.0f,  "Thickness of the curved line",          true)    \
X(swellFactor,       float,     1.5f,   0.0f,   10.0f,   "Swelling multiplier at midpoint",       true)    \
X(easeDownInSeconds, float,     1.0f,   0.01f,  10.0f,   "Ease-out fade duration (seconds)",      true)    \
X(useParticleColors, bool,      true,   0,      1,       "Use original particle colors",          true)    \
X(lineColor,         sf::Color, sf::Color(255,255,255,255), 0, 255, "Primary fallback line color", false)  \
X(otherLineColor,    sf::Color, sf::Color(255,255,255,255), 0, 255, "Alternate/fading line color", false)  \
X(invertColorTime,   bool,      false,  0,      1,       "Colors fade in over time rather than out", true) \
X(curvature,         float,     0.25f,  -NX_PI,  NX_PI,  "Amount of curvature (arc)",             true)    \
X(lineSegments,      int32_t,   20,     1,      200,     "Number of segments in the curve",       true)

    struct SeqLineData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(PARTICLE_LINE_MODIFIER_PARAMS)
    };

    enum class E_SeqLineModifierParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(PARTICLE_LINE_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_SeqLineModifierParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(PARTICLE_LINE_MODIFIER_PARAMS)
    };

  public:
    explicit ParticleSequentialLineModifier( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(PARTICLE_LINE_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    void drawMenu() override;

    bool isActive() const override { return m_isActive; }

    void processMidiEvent(const Midi_t &midiEvent) override {}

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    E_ModifierType getType() const override { return E_ModifierType::E_SequentialModifier; }

    void update( const sf::Time &deltaTime ) override {}

    void modify(
       const sf::BlendMode& blendMode,
       std::deque< IParticle* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override;

  private:

    PipelineContext& m_ctx;
    bool m_isActive { true };
    SeqLineData_t m_data;

  };

}