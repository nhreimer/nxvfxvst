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

#include "models/shader/SmearShader.hpp"

namespace nx
{
  SmearShader::SmearShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load smear fragment shader" );
    }
    else
    {
      LOG_INFO( "Smear fragment shader loaded" );
    }

    EXPAND_SHADER_VST_BINDINGS(SMEAR_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  SmearShader::~SmearShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  [[nodiscard]]
  nlohmann::json SmearShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(SMEAR_SHADER_PARAMS)

    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void SmearShader::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(SMEAR_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void SmearShader::trigger(const Midi_t &midi)
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
    {
      m_easing.trigger();
    }
  }

  void SmearShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Smear Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(SMEAR_SHADER_PARAMS, m_data)

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  [[nodiscard]]
  bool SmearShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * SmearShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    if ( !m_feedbackTexture.isInitialized() )
    {
      m_feedbackTexture.ensureSize( inputTexture->getSize() );
      // we only want to draw this once
      m_feedbackTexture.clear( sf::Color::Black );
      m_feedbackTexture.display();
    }
    else
      m_feedbackTexture.ensureSize( inputTexture->getSize() );

    const float easing = m_easing.getEasing();

    m_shader.setUniform("texture", inputTexture->getTexture());
    m_shader.setUniform("resolution", sf::Vector2f(inputTexture->getSize() ) );
    m_shader.setUniform("smearLength", m_data.length.first);
    m_shader.setUniform("smearIntensity", m_data.intensity.first);
    m_shader.setUniform("sampleCount", m_data.sampleCount.first);

    m_shader.setUniform("time", m_clock.getElapsedTime().asSeconds());
    m_shader.setUniform("jitterAmount", m_data.jitterAmount.first);         // 0.0–0.2
    m_shader.setUniform("brightnessBoost", m_data.brightnessBoost.first);   // 1.0–3.0
    m_shader.setUniform("pulseValue", easing);
    m_shader.setUniform("falloffPower", m_data.falloffPower.first);         // e.g. 1.0 = linear, >1 = tighter fade
    m_shader.setUniform("brightnessPulse", easing);

    m_shader.setUniform("directionAngle", m_data.directionAngleInRadians.first);
    m_shader.setUniform("wiggleAmplitude", m_data.wiggleAmplitude.first);
    m_shader.setUniform("wiggleFrequency", m_data.wiggleFrequency.first);

     const auto tintVec = sf::Glsl::Vec3(
         static_cast< float >(m_data.tint.first.r) / 255.f,
         static_cast< float >(m_data.tint.first.g) / 255.f,
         static_cast< float >(m_data.tint.first.b) / 255.f
     );

    m_shader.setUniform("smearTint", tintVec);

    // 1. Use the previous feedback frame as input
    const sf::Sprite feedbackSprite( m_feedbackTexture.getTexture() );

    // 2. Apply the smear shader TO the feedback (draw into m_outputTexture)
    m_outputTexture.clear();
    m_outputTexture.draw(feedbackSprite, &m_shader);
    m_outputTexture.display();

    // 3. Fade feedback with a semi-transparent black quad to prevent infinite trails
    m_feedbackFadeShape.setFillColor(
      sf::Color(0, 0, 0,
                  static_cast< uint8_t >( 255 * m_data.feedbackFade.first ) ) );

    m_feedbackTexture.draw(m_feedbackFadeShape, sf::BlendAlpha);

    // 4. Add current smeared frame into feedback buffer
    const sf::Sprite smearedFrame(m_outputTexture.getTexture());
    m_feedbackTexture.draw(smearedFrame, m_data.feedbackBlendMode.first);

    // 5. Display feedback buffer
    m_feedbackTexture.display();

    // 6. Output the feedback as final result
    //return m_feedbackTexture;
    return m_blender.applyShader( inputTexture,
                            m_feedbackTexture.get(),
                                  m_data.mixFactor.first );
  }

}