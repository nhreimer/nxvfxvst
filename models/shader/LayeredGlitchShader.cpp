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

#include "models/shader/LayeredGlitchShader.hpp"

namespace nx
{
     LayeredGlitchShader::LayeredGlitchShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load glitch fragment shader" );
      }
      else
      {
        LOG_INFO( "Loaded glitch fragment shader" );
      }

      EXPAND_SHADER_VST_BINDINGS(GLITCH_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    LayeredGlitchShader::~LayeredGlitchShader()
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json LayeredGlitchShader::serialize() const
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(GLITCH_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_burstManager.serialize();
      return j;
    }

    void LayeredGlitchShader::deserialize( const nlohmann::json& j )
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(GLITCH_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_burstManager.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    ///////////////////////////////////////////////////////
    /// IMENUABLE
    ///////////////////////////////////////////////////////

    void LayeredGlitchShader::drawMenu()
    {
      if ( ImGui::TreeNode( "Glitch Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        EXPAND_SHADER_IMGUI(GLITCH_SHADER_PARAMS, m_data)

        ImGui::Separator();
        m_burstManager.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    ///////////////////////////////////////////////////////
    /// ISHADER
    ///////////////////////////////////////////////////////

    void LayeredGlitchShader::update( const sf::Time &deltaTime )
    {
      m_burstManager.update( deltaTime.asSeconds() );
    }

    void LayeredGlitchShader::trigger( const Midi_t& midi )
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      {
        m_burstManager.trigger();
        m_clock.restart();
      }
    }

    [[nodiscard]]
    bool LayeredGlitchShader::isShaderActive() const { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture * LayeredGlitchShader::applyShader( const sf::RenderTexture * inputTexture )
    {
      m_outputTexture.ensureSize( inputTexture->getSize() );

      const float cumulative = m_burstManager.getEasing();
      const float boostedStrength = m_data.glitchBaseStrength.first + cumulative * m_data.glitchPulseBoost.first;

      m_shader.setUniform("glitchStrength", boostedStrength);
      //m_shader.setUniform("easingValue", cumulative); // optional, for shader-side sync

      // determine whether to apply cumulative triggers only, which provides a staccato feel, especially
      // on fast beats, but it can be weird on slower events
      if ( !m_data.applyOnlyOnEvents.first )
        m_shader.setUniform("easedTime", m_clock.getElapsedTime().asSeconds() );
      else
        m_shader.setUniform("easedTime", m_burstManager.getLastTriggeredInSeconds() );

      m_shader.setUniform("glitchStrength", boostedStrength);

      m_shader.setUniform("texture", inputTexture->getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture->getSize()));

      m_shader.setUniform("glitchAmount", m_data.glitchAmount.first);
      // m_shader.setUniform("scanlineIntensity", m_data.scanlineIntensity);
      m_shader.setUniform("chromaFlickerAmount", m_data.chromaFlickerAmount.first);
      m_shader.setUniform("pixelJumpAmount", m_data.pixelJumpAmount.first);
      m_shader.setUniform("bandCount", m_data.bandCount.first);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture->getTexture()), &m_shader);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                              m_outputTexture.get(),
                              m_data.mixFactor.first );
    }

}