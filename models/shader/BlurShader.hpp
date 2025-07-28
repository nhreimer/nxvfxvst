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
#include "models/shader/BlenderShader.hpp"
#include "vst/VSTStateContext.hpp"

namespace nx
{

  class BlurShader final : public IShader
  {

#define BLUR_SHADER_PARAMS(X)                                                                     \
X(sigma,             float, 7.f,     0.f,   50.f , "Amount of blurring", true)                    \
X(brighten,          float, 1.f,     1.f,   5.f  , "Brightens the blurred areas", true)           \
X(blurHorizontal,    float, 1.0f,    0.f,   20.f , "Blurs in the horizontal direction", true)     \
X(blurVertical,      float, 1.0f,    0.f,   20.f , "Blurs in the vertical direction", true)       \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct BlurData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(BLUR_SHADER_PARAMS)
    };

    enum class E_BlurParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(BLUR_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_BlurParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(BLUR_SHADER_PARAMS)
    };

  public:

    explicit BlurShader( PipelineContext& context );

    ~BlurShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_intermediary.destroy();
      m_blender.destroyTextures();
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    // identify type for easier loading
    E_ShaderType getType() const override { return E_ShaderType::E_BlurShader; }

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
    sf::RenderTexture * applyShader(
      const sf::RenderTexture * inputTexture ) override;

  private:

    PipelineContext& m_ctx;

    sf::Shader m_shader;
    LazyTexture m_intermediary;
    LazyTexture m_outputTexture;

    BlurData_t m_data;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float blurRadiusX;
uniform float blurRadiusY;
uniform vec2 direction;
uniform float sigma;
uniform float brighten;
uniform float intensity; // Amplifies both blur and brightness
uniform float decay;

// Gaussian function
float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.50662827463 * sigma);
}

void main() {
    vec2 texSize = textureSize(texture, 0);

    // Apply intensity to sigma (blur)
    float amplifiedSigma = sigma * (1.0 + intensity);
    vec2 texOffset = direction * vec2(blurRadiusX / texSize.x, blurRadiusY / texSize.y);

    // Center sample
    vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gaussian(0.0, amplifiedSigma);
    float totalWeight = gaussian(0.0, amplifiedSigma);

    // Blur samples
    for (int i = 1; i <= 10; i++) {
        float offset = float(i);
        float weight = gaussian(offset, amplifiedSigma);

        color += texture2D(texture, gl_TexCoord[0].xy + texOffset * offset) * weight;
        color += texture2D(texture, gl_TexCoord[0].xy - texOffset * offset) * weight;
        totalWeight += 2.0 * weight;
    }

    // Apply brighten + intensity boost
    float amplifiedBrighten = brighten + intensity;

    gl_FragColor = ( color * amplifiedBrighten ) / totalWeight;
})";

  };
}