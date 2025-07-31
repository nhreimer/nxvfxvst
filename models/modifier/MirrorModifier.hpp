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

namespace nx
{
  /// this is a class fleshed out for testing purposes only
  class MirrorModifier final : public IParticleModifier
  {

#define MIRROR_MODIFIER_PARAMS(X)                                                                           \
X(count,              int32_t,   4,     1,   64,   "Number of mirrored instances",           true)        \
X(distance,           float,     50.0f, 0.0f, 1000.0f, "Distance from original particle",    true)        \
X(mirrorAlpha,        float,     1.0f,  0.0f, 1.0f,   "Alpha transparency for mirrors",       true)       \
X(lastVelocityNorm,   float,     0.0f,  0.0f, 1.0f,   "Normalized velocity (readonly?)",     true)        \
X(useDynamicRadius,   bool,      false, 0,    1,     "Enable dynamic radius scaling",        true)        \
X(dynamicRadius,      float,     1.0f,  0.0f, 10.0f,  "Dynamic radius scale factor",         true)        \
X(angleOffsetDegrees, float,     0.0f, -360.0f, 360.0f, "Rotation offset per mirror",       true)         \
X(useParticleColors,  bool,      false, 0,    1,     "Use original particle colors",         true)        \
X(mirrorColor,        sf::Color, sf::Color::White, 0, 255, "Fallback mirror color",          false)       \
X(mirrorOutlineColor, sf::Color, sf::Color::White, 0, 255, "Outline color for mirrors",      false)


    struct MirrorData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(MIRROR_MODIFIER_PARAMS)
    };

    enum class E_MirrorModifierParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(MIRROR_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_MirrorModifierParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(MIRROR_MODIFIER_PARAMS)
    };

  public:
    explicit MirrorModifier( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(MIRROR_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    E_ModifierType getType() const override { return E_ModifierType::E_MirrorModifier; }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    void drawMenu() override;

    void update(const sf::Time &) override {}

    [[nodiscard]]
    bool isActive() const override { return m_data.isActive; }

    void processMidiEvent(const Midi_t & midi) override
    {
      m_data.lastVelocityNorm.first = midi.velocity;
    }

    void modify(const sf::BlendMode& blendMode,
                std::deque< IParticle * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override;

  private:

    PipelineContext& m_ctx;
    MirrorData_t m_data;

  };

} // namespace nx
