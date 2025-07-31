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

#include "models/shader/BlenderShader.hpp"
#include "helpers/CommonHeaders.hpp"
#include "helpers/SerialHelper.hpp"

#include "models/IShader.hpp"
#include "models/data/PipelineContext.hpp"
#include "models/easings/TimeEasing.hpp"

#include "shapes/MidiNoteControl.hpp"

#include "utils/LazyTexture.hpp"

namespace nx
{

  class StrobeShader final : public IShader
  {

#define STROBE_SHADER_PARAMS(X)                                                                      \
X(flashAmount,  float, 20.f, 1.f, 100.f, "Speed of strobe pulses (Hz)", true)                        \
X(flashColor,   sf::Color, sf::Color::White, 0.f, 0.f, "Flash color applied during strobe", true)    \
X(mixFactor,    float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct StrobeData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(STROBE_SHADER_PARAMS)
    };

    enum class E_StrobeParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(STROBE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_StrobeParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(STROBE_SHADER_PARAMS)
    };

  public:
    explicit StrobeShader( PipelineContext& context );

    ~StrobeShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    E_ShaderType getType() const override { return E_ShaderType::E_StrobeShader; }

    void drawMenu() override;

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader( const sf::RenderTexture * inputTexture ) override;

  private:
    PipelineContext& m_ctx;
    StrobeData_t m_data;

    sf::Shader m_shader;
    LazyTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float flashAmount;     // 0.0 = normal scene, 1.0 = full flash
uniform vec4 flashColor;       // user-defined color

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(texture, 0));
    vec4 base = texture2D(texture, uv);

    // Blend ENTIRE scene toward flashColor based on flashAmount
    vec3 finalColor = mix(base.rgb, flashColor.rgb, flashAmount);

    gl_FragColor = vec4(finalColor, flashColor.a);
})";
  };
}