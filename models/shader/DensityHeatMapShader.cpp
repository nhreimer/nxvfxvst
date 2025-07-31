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

#include "models/shader/DensityHeatMapShader.hpp"

#include "helpers/ColorHelper.hpp"
#include "helpers/SerialHelper.hpp"

namespace nx
{

  DensityHeatMapShader::DensityHeatMapShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load density heat map fragment shader" );
    }
    else
    {
      LOG_INFO( "Loaded density heat map fragment shader" );
    }

    EXPAND_SHADER_VST_BINDINGS(DENSITY_HEATMAP_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json DensityHeatMapShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum(getType());
    EXPAND_SHADER_PARAMS_TO_JSON(DENSITY_HEATMAP_SHADER_PARAMS)
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void DensityHeatMapShader::deserialize(const nlohmann::json& j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(DENSITY_HEATMAP_SHADER_PARAMS)
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void DensityHeatMapShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Density Heat Map" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(DENSITY_HEATMAP_SHADER_PARAMS, m_data)

      ImGui::SeparatorText( "Easings" );
      m_easing.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void DensityHeatMapShader::trigger( const Midi_t &midi )
  {
    m_easing.trigger();
  }

  [[nodiscard]]
  bool DensityHeatMapShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * DensityHeatMapShader::applyShader( const sf::RenderTexture * inputTexture )
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    m_shader.setUniform("u_densityTexture", inputTexture->getTexture());
    m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture->getSize() });
    m_shader.setUniform("u_falloff", m_data.falloff.first * m_easing.getEasing() );

    m_outputTexture.clear( sf::Color::Transparent );

    m_shader.setUniform( "u_colorCoolStart", ColorHelper::convertFromVec4( m_data.colorCoolStart.first ) );
    m_shader.setUniform( "u_colorCoolEnd", ColorHelper::convertFromVec4( m_data.colorCoolEnd.first ) );

    m_shader.setUniform( "u_colorWarmStart", ColorHelper::convertFromVec4( m_data.colorWarmStart.first ) );
    m_shader.setUniform( "u_colorWarmEnd", ColorHelper::convertFromVec4( m_data.colorWarmEnd.first ) );

    m_shader.setUniform( "u_colorHotStart", ColorHelper::convertFromVec4( m_data.colorHotStart.first ) );
    m_shader.setUniform( "u_colorHotEnd", ColorHelper::convertFromVec4( m_data.colorHotEnd.first ) );

    m_shader.setUniform( "u_colorMaxStart", ColorHelper::convertFromVec4( m_data.colorMaxStart.first ) );
    m_shader.setUniform( "u_colorMaxEnd", ColorHelper::convertFromVec4( m_data.colorMaxEnd.first ) );

    m_outputTexture.draw( sf::Sprite( inputTexture->getTexture() ), &m_shader );
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                            m_outputTexture.get(),
                            m_data.mixFactor.first );
  }

}