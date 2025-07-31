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

#include "models/IShader.hpp"
#include "models/data/PipelineContext.hpp"
#include "models/shader/BlenderShader.hpp"

#include "helpers/CommonHeaders.hpp"
#include "helpers/SerialHelper.hpp"

#include "models/easings/TimeEasing.hpp"

#include "shapes/MidiNoteControl.hpp"
#include "shapes/TimedCursorPosition.hpp"

#include "utils/LazyTexture.hpp"

namespace nx
{

  class ShockBloomShader final : public IShader
  {

#define SHOCK_BLOOM_SHADER_PARAMS(X)                                                                         \
X(center,            sf::Vector2f, sf::Vector2f(0.5f, 0.5f), 0.f, 0.f, "Center of the ring (UV)", false)     \
X(color,             sf::Glsl::Vec3, sf::Glsl::Vec3(1.f, 1.f, 1.f), 0.f, 0.f, "Shock ring RGB color", false) \
X(maxRadius,         float, 0.6f,   0.1f, 1.5f,  "Maximum radius of the ring", true)                        \
X(thickness,         float, 0.05f,  0.01f, 0.2f, "Ring thickness (falloff)", true)                          \
X(intensity,         float, 2.0f,   0.0f, 5.0f,  "Glow strength of the ring", true)                         \
X(easingMultiplier,  float, 1.0f,   0.0f, 5.0f,  "How strongly the easing drives visibility", true)         \
X(innerTransparency, float, 1.0f, 0.f, 1.f, "Transparency multiplier for the center of the ring", true)     \
X(mixFactor,         float, 1.0f, 0.f, 1.f, "Mix between original and effects result", true)                \
X(BlendInput,        sf::BlendMode, sf::BlendAdd, 0.f, 0.f, nullptr, false )

    struct ShockBloomData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(SHOCK_BLOOM_SHADER_PARAMS)
    };

    enum class E_ShockBloomParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(SHOCK_BLOOM_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_ShockBloomParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(SHOCK_BLOOM_SHADER_PARAMS)
    };

  public:

    explicit ShockBloomShader(PipelineContext& context);

    ~ShockBloomShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json& j) override;

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_ShockBloomShader; }

    void update( const sf::Time& deltaTime ) override
    {}

    void trigger( const Midi_t& midi ) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    void drawMenu() override;

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader( const sf::RenderTexture * inputTexture ) override;

  private:

    PipelineContext& m_ctx;
    ShockBloomData_t m_data;

    sf::Shader m_shader;
    LazyTexture m_outputTexture;

    BlenderShader m_blender;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    TimedCursorPosition m_timedCursor;

    inline static const std::string m_fragmentShader = R"(uniform vec2 resolution;
uniform vec2 center;
uniform float radius;
uniform float thickness;
uniform vec3 color;
uniform float intensity;
uniform float innerTransparency;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    float dist = length(uv - center);

    // Ring falloff
    float edgeFade = smoothstep(radius, radius - thickness, dist) *
                     smoothstep(radius + thickness, radius, dist);

    // Inner transparency falloff
    float innerCutout = smoothstep(0.0, radius - thickness * 0.5, dist); // transparent inside
    float ringShape = edgeFade * innerCutout;

    vec3 glow = color * ringShape * intensity;

    gl_FragColor = vec4(glow, ringShape * innerTransparency); // final transparency applied
})";

  };

}