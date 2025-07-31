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

  class SmearShader final : public IShader
  {
#define SMEAR_SHADER_PARAMS(X)                                                                       \
X(directionAngleInRadians, float, 0.f,  -NX_PI, NX_PI,  "Angle of smear direction (in radians)", true)     \
X(length,                 float, 0.2f,  0.f,  1.f,     "Length of smear trail (0 = off, 1 = full)", true)  \
X(intensity,              float, 0.5f,  0.f,  1.f,     "Blend amount with previous frame", true)           \
X(tint,                   sf::Color, sf::Color(255,255,255), 0.f, 0.f, "Optional color overlay", false)     \
X(sampleCount,            int,   32,    1,   128,      "Number of smear samples (quality vs speed)", true) \
X(jitterAmount,           float, 0.f,   0.f,  1.f,     "Randomness in smear direction per sample", true)   \
X(brightnessBoost,        float, 1.f,   0.f,  10.f,    "Overall brightness multiplier", true)              \
X(brightnessPulse,        float, 1.f,   0.f,  5.f,     "Brightness wave amount during pulses", true)       \
X(falloffPower,           float, 1.f,   0.1f, 10.f,    "Exponential falloff on smear fade", true)          \
X(wiggleAmplitude,        float, 0.f,   0.f,  NX_PI,   "Wiggle amount (radians) per sample", true)         \
X(wiggleFrequency,        float, 0.f,   0.f,  50.f,    "Wiggle speed (Hz)", true)                          \
X(feedbackFade,           float, 0.05f, 0.f,  1.f,     "Fadeout amount for feedback trail", true)          \
X(feedbackBlendMode,      sf::BlendMode, sf::BlendAdd, 0, 0, "Blend mode used for feedback drawing", false) \
X(feedbackRotation,       float, 0.f,   -360.f, 360.f, "Rotational offset added to feedback frame", true)  \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct SmearData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(SMEAR_SHADER_PARAMS)
    };

    enum class E_SmearParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(SMEAR_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_SmearParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(SMEAR_SHADER_PARAMS)
    };

  public:

    explicit SmearShader( PipelineContext& context );

    ~SmearShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_feedbackTexture.destroy();
      m_blender.destroyTextures();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_SmearShader; };

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    void drawMenu() override;

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:
    PipelineContext& m_ctx;

    SmearData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;

    LazyTexture m_outputTexture;
    LazyTexture m_feedbackTexture;

    BlenderShader m_blender;
    TimeEasing m_easing;
    MidiNoteControl m_midiNoteControl;

    sf::RectangleShape m_feedbackFadeShape;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;

uniform float smearLength;
uniform float smearIntensity;
uniform vec3 smearTint;
uniform int sampleCount;

uniform float jitterAmount;
uniform float brightnessBoost;
uniform float brightnessPulse;
uniform float pulseValue;
uniform float falloffPower;

uniform float directionAngle;
uniform float wiggleAmplitude;
uniform float wiggleFrequency;
uniform float time;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec4 baseColor = texture2D(texture, uv);
    vec4 smearColor = vec4(0.0);

    //vec2 dir = normalize(smearDirection);

    // wiggle, wiggle, baby!
    float wiggle = sin(time * wiggleFrequency) * wiggleAmplitude;
    float angle = directionAngle + wiggle;
    vec2 dir = vec2(cos(angle), sin(angle));

    for (int i = 1; i <= sampleCount; ++i) {
        float t = float(i) / float(sampleCount);
        float falloff = pow(1.0 - t, falloffPower); // falloff control

        // Jitter
        float angle = hash(t + time) * 6.28318; // random angle
        vec2 jitter = vec2(cos(angle), sin(angle)) * jitterAmount * t;

        vec2 offset = -dir * smearLength * t + jitter;
        vec4 sample = texture2D(texture, uv + offset);
        sample.rgb *= smearTint * falloff;
        smearColor += sample;
    }

    smearColor /= float(sampleCount);

    // Intensity blend
    vec4 blended = mix(baseColor, smearColor, clamp(smearIntensity + pulseValue, 0.0, 1.0));

    // Brightness boost (final optional pop)
    //blended.rgb *= brightnessBoost;
    blended.rgb *= brightnessBoost + brightnessPulse;

    gl_FragColor = blended;
})";

  };

}