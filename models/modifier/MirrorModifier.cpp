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

#include "models/modifier/MirrorModifier.hpp"

namespace nx
{
  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json MirrorModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(MIRROR_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(MIRROR_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Mirror Modifier" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(MIRROR_MODIFIER_PARAMS, m_data)

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::modify(const sf::BlendMode& blendMode,
              std::deque< IParticle * > &particles,
              std::deque< sf::Drawable * > &outArtifacts)
  {
    for (const auto* p : particles)
    {
      const sf::Vector2f origin = p->getPosition();
      const float baseAngle =
        std::atan2(origin.y - m_ctx.globalInfo.windowHalfSize.y, origin.x - m_ctx.globalInfo.windowHalfSize.x);
      const float angleOffsetRad = m_data.angleOffsetDegrees.first * NX_D2R;

      const float radius = ( m_data.useDynamicRadius.first )
        ? m_data.distance.first * m_data.dynamicRadius.first * m_data.lastVelocityNorm.first
        : m_data.distance.first;

      for (int i = 0; i < m_data.count.first; ++i)
      {
        const float angle = baseAngle + angleOffsetRad + i * (2.f * NX_PI / m_data.count.first);
        const sf::Vector2f mirrorPos =
        {
          origin.x + std::cos(angle) * radius,
          origin.y + std::sin(angle) * radius
        };

        auto* shape = p->clone( m_ctx.globalInfo.elapsedTimeSeconds ); // copy
        auto fillColors = m_data.useParticleColors.first
          ? shape->getColors()
          : std::make_pair( m_data.mirrorColor.first, m_data.mirrorColor.first );

        fillColors.first.a = static_cast< uint8_t >(fillColors.first.a * m_data.mirrorAlpha.first);
        fillColors.second.a = static_cast< uint8_t >(fillColors.first.a * m_data.mirrorAlpha.first);
        shape->setColorPattern(fillColors.first, fillColors.second);

        if ( shape->getOutlineThickness() > 0.f )
        {
          auto outlineColors = m_data.useParticleColors.first
            ? shape->getOutlineColors()
            : std::make_pair( m_data.mirrorOutlineColor.first, m_data.mirrorOutlineColor.first );

          outlineColors.first.a = static_cast< uint8_t >(outlineColors.first.a * m_data.mirrorAlpha.first);
          outlineColors.second.a = static_cast< uint8_t >(outlineColors.second.a * m_data.mirrorAlpha.first);
          shape->setOutlineColorPattern(outlineColors.first, outlineColors.second);
        }

        shape->setPosition(mirrorPos);
        outArtifacts.push_back(shape); // user doesn't own memory
      }
    }
  }
} // namespace nx
