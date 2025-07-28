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

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class DualKawaseBlurShader final : public IShader
  {

    // The BlenderShader for this one is already built in. it was the prototype example.
#define DUALKAWASE_SHADER_PARAMS(X)                                                                 \
X(passes,      int,   4,     1,    10,    "Number of downsample/upsample passes", true)             \
X(offset,      float, 1.0f,  0.0f, 10.f,  "Kernel offset per pass",true)                            \
X(bloomGain,   float, 1.0f,  0.0f, 10.f,  "Gain applied to bloom texture before blending", true)    \
X(brightness,  float, 1.0f,  0.0f, 3.f,   "Brightness boost for final output", true)                \
X(mixFactor,   float, 1.0f,  0.0f, 1.f,   "Blend factor between base and blurred result", true)

    struct DKBlurData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(DUALKAWASE_SHADER_PARAMS)
    };

    enum class E_DualKawaseParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(DUALKAWASE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_DualKawaseParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(DUALKAWASE_SHADER_PARAMS)
    };

  public:

    explicit DualKawaseBlurShader( PipelineContext& context );

    ~DualKawaseBlurShader() override;

    void destroyTextures() override
    {
      m_pingTexture.destroy();
      m_pongTexture.destroy();
      m_compositeTexture.destroy();
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    // identify type for easier loading
    E_ShaderType getType() const override { return E_ShaderType::E_DualKawaseBlurShader; }

    void update( const sf::Time& deltaTime ) override {}

    void drawMenu() override;

    void trigger( const Midi_t& midi ) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:

    PipelineContext& m_ctx;

    sf::Shader m_shader;
    sf::Shader m_compositeShader;

    // ping-pong texture strategy for n passes + composite for mixing
    LazyTexture m_pingTexture;
    LazyTexture m_pongTexture;
    LazyTexture m_compositeTexture;

    DKBlurData_t m_data;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_texelSize;
uniform float u_offset;
uniform float u_bloomGain;     // global gain
uniform float u_brightness;    // base boost

void main() {
    vec2 uv = gl_FragCoord.xy * u_texelSize;

    vec3 color = texture2D(u_texture, uv).rgb * 4.0;
    color += texture2D(u_texture, uv + u_texelSize * vec2( u_offset,  u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2(-u_offset,  u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2( u_offset, -u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2(-u_offset, -u_offset)).rgb;

    color = color / 8.0;

    // BOOST!
    color *= u_brightness;
    color *= u_bloomGain;

    gl_FragColor = vec4(color, 1.0);
})";

    inline static const std::string m_compositeFragmentShader = R"(uniform sampler2D u_scene;     // original render
uniform sampler2D u_bloom;     // blurred bloom texture
uniform float u_mixFactor;     // 0.0 = off, 1.0 = full blend, >1.0 = glow boost

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_scene, 0));

    vec3 sceneColor = texture2D(u_scene, uv).rgb;
    vec3 bloomColor = texture2D(u_bloom, uv).rgb;

    // Blend bloom back into the original
    vec3 finalColor = mix(sceneColor, sceneColor + bloomColor, u_mixFactor);

    gl_FragColor = vec4(finalColor, 1.0);
}
)";

  };
}