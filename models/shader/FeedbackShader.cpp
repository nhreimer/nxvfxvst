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

#include "models//shader/FeedbackShader.hpp"

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  FeedbackShader::FeedbackShader( PipelineContext& context )
    : m_ctx( context )
  {
    EXPAND_SHADER_VST_BINDINGS(FEEDBACK_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  FeedbackShader::~FeedbackShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json FeedbackShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum(getType());
    EXPAND_SHADER_PARAMS_TO_JSON(FEEDBACK_SHADER_PARAMS)
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void FeedbackShader::deserialize(const nlohmann::json& j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(FEEDBACK_SHADER_PARAMS)
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      m_easing.setEasingType( E_EasingType::E_Disabled );
    }
  }

  void FeedbackShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Feedback Options" ) )
    {
      EXPAND_SHADER_IMGUI(FEEDBACK_SHADER_PARAMS, m_data)

      if ( ImGui::SmallButton( "Clear" ) )
        m_outputTexture.clear( sf::Color::Transparent );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void FeedbackShader::trigger( const Midi_t &midi )
  {
    m_easing.trigger();
  }

  [[nodiscard]]
  bool FeedbackShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * FeedbackShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    const auto targetSize = sf::Vector2f{ inputTexture->getSize() };

    const auto clampedEasing = std::clamp( m_easing.getEasing(), 0.f, 1.f );

    // Resize fade quad
    m_fadeQuad.setSize(targetSize);
    m_fadeQuad.setFillColor(sf::Color(m_data.fadeColor.first.r,
                                          m_data.fadeColor.first.g,
                                          m_data.fadeColor.first.b,
                                          static_cast<uint8_t>(m_data.trailFadeAlpha.first * clampedEasing)));

    // Step 1: Fade out previous trail
    m_outputTexture.draw(m_fadeQuad, sf::BlendAlpha);

    // Step 2: Draw new input frame on top
    m_outputTexture.draw(sf::Sprite(inputTexture->getTexture()), sf::BlendAdd);

    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                                  m_outputTexture.get(),
                                  m_data.mixFactor.first );
  }

}