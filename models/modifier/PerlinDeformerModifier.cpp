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

#include "models/modifier/PerlinDeformerModifier.hpp"

#include "helpers/SerialHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////

  [[nodiscard]]
  nlohmann::json PerlinDeformerModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(PERLIN_DEFORMER_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(PERLIN_DEFORMER_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Perlin Deformer" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(PERLIN_DEFORMER_MODIFIER_PARAMS, m_data)

      ImGui::SeparatorText( "Perlin Deformer Types" );
      if ( ImGui::RadioButton( "Hash##1", m_data.noiseType == E_NoiseType::E_Hash ) )
      {
        m_data.noiseType = E_NoiseType::E_Hash;
      }
      else if ( ImGui::RadioButton( "Value##1", m_data.noiseType == E_NoiseType::E_Value ) )
      {
        m_data.noiseType = E_NoiseType::E_Value;
      }
      else if ( ImGui::RadioButton( "FBM##1", m_data.noiseType == E_NoiseType::E_FBM ) )
      {
        m_data.noiseType = E_NoiseType::E_FBM;
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::modify(
     const sf::BlendMode& blendMode,
     std::deque< IParticle* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
  {
    for (size_t i = 0; i < particles.size(); ++i)
    {
      const sf::Vector2f pos = particles[ i ]->getPosition();
      const float x = pos.x * m_data.noiseScale.first;
      const float y = pos.y * m_data.noiseScale.first;

      const float offsetX = (getNoise(x + m_time, y) - 0.5f) * 2.f * m_data.deformStrength.first;
      const float offsetY = (getNoise(x, y + m_time) - 0.5f) * 2.f * m_data.deformStrength.first;

      const sf::Vector2f warpedPos = pos + sf::Vector2f(offsetX, offsetY);

      // don't get stuck in an infinite loop
      //auto *copiedParticle = particles.emplace_back(new TimedParticle_t(*particles[ i ]));
      //copiedParticle->shape.setPosition(warpedPos);
      // auto * copiedShape = dynamic_cast< CircleParticle * >(
      //   outArtifacts.emplace_back( new CircleParticle( particles[ i ]->shape ) ) );

      auto * copiedShape = dynamic_cast< IParticle* >(
        outArtifacts.emplace_back( particles[ i ]->clone( m_ctx.globalInfo.elapsedTimeSeconds ) ) );

      copiedShape->setPosition( warpedPos );
      if ( m_data.useParticleColors.first )
      {
        const auto colors = copiedShape->getColors();
        copiedShape->setColorPattern(
          { colors.first.r, colors.first.g, colors.first.b, static_cast< uint8_t >(colors.first.a * m_data.colorFade.first) },
  { colors.second.r, colors.second.g, colors.second.b, static_cast< uint8_t >(colors.second.a * m_data.colorFade.first) } );
      }
      else
      {
        const auto& color = m_data.perlinColor.first;
        copiedShape->setColorPattern( { color.r, color.g, color.b, static_cast< uint8_t >(color.a * m_data.colorFade.first) },
          { color.r, color.g, color.b, static_cast< uint8_t >(color.a * m_data.colorFade.first) } );
      }
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  float PerlinDeformerModifier::getNoise( const float x, const float y ) const
  {
    switch ( m_data.noiseType )
    {
      case E_NoiseType::E_Hash:  return getHashNoise(x, y);
      case E_NoiseType::E_Value: return getValueNoise(x, y);
      case E_NoiseType::E_FBM:   return getFBM(x, y, m_data.octaves.first);
      default: return 0.f;
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // hash noise: fast but low quality
  float PerlinDeformerModifier::getHashNoise(float x, float y)
  {
    const float dotVal = x * 12.9898f + y * 78.233f;
    const float sinVal = std::sin(dotVal) * 43758.5453f;
    return sinVal - std::floor(sinVal); // now in [0.0, 1.0)
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // Smooth blend between grid points
  // Simple 2D value noise with bilinear interpolation
  float PerlinDeformerModifier::getValueNoise( const float x, const float y)
  {
    const int xi = static_cast<int>(std::floor(x));
    const int yi = static_cast<int>(std::floor(y));
    const float xf = x - xi;
    const float yf = y - yi;

    const float tl = getHashNoise(xi, yi);
    const float tr = getHashNoise(xi + 1, yi);
    const float bl = getHashNoise(xi, yi + 1);
    const float br = getHashNoise(xi + 1, yi + 1);

    const float top = lerp(tl, tr, xf);
    const float bottom = lerp(bl, br, xf);

    return lerp(top, bottom, yf); // [0,1]
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // Fractal Brownian Motion
  // Layered noise — adds complexity and control
  float PerlinDeformerModifier::getFBM( const float x, const float y, const int octaves)
  {
    float value = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;

    for (int i = 0; i < octaves; ++i)
    {
      value += amplitude * getValueNoise(x * frequency, y * frequency);
      frequency *= 2.0f;
      amplitude *= 0.5f;
    }

    return value;
  }

}