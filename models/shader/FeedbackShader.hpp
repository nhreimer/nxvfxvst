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

namespace nx
{

  /// not actually a shader. it's a utility to be inserted anywhere in the shader chain.
  class FeedbackShader final : public IShader
  {

#define FEEDBACK_SHADER_PARAMS(X)                                                                   \
X(trailFadeAlpha, int, 8,   0,   255,  "Alpha value subtracted from each trail frame", true)        \
X(fadeColor,      sf::Color, sf::Color::Black, 0, 0,  "Color applied during trail fading", true)    \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct FeedbackData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(FEEDBACK_SHADER_PARAMS)
    };

    enum class E_FeedbackParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(FEEDBACK_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FeedbackParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(FEEDBACK_SHADER_PARAMS)
    };

  public:
    explicit FeedbackShader( PipelineContext& context );

    ~FeedbackShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json& j) override;

    E_ShaderType getType() const override { return E_ShaderType::E_FeedbackShader; }

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
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:
    PipelineContext& m_ctx;
    FeedbackData_t m_data;

    sf::RectangleShape m_fadeQuad;

    sf::Shader m_shader;
    //sf::RenderTexture m_outputTexture;
    LazyTexture m_outputTexture;

    BlenderShader m_blender;

    TimeEasing m_easing;

  };
}