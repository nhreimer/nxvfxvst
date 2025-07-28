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

#include "models/modifier/ParticleSequentialLineModifier.hpp"

#include "helpers/LineHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  void ParticleSequentialLineModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Sequential Lines" ) )
    {
      EXPAND_SHADER_IMGUI(PARTICLE_LINE_MODIFIER_PARAMS, m_data)

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  nlohmann::json ParticleSequentialLineModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(PARTICLE_LINE_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleSequentialLineModifier::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(PARTICLE_LINE_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleSequentialLineModifier::modify(
     const sf::BlendMode& blendMode,
     std::deque< IParticle* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
  {
    for ( int i = 0; i < particles.size(); ++i )
    {
      if ( m_data.isActive && i > 0 )
      {
        auto * line = dynamic_cast< CurvedLine* >( outArtifacts.emplace_back(
          new CurvedLine( particles[ i - 1 ]->getPosition(),
            particles[ i ]->getPosition(),
            m_data.curvature.first,
            m_data.lineSegments.first ) ) );

        line->setWidth( m_data.lineThickness.first );

        if ( m_data.useParticleColors.first )
        {
          LineHelper::updateLineColors( line,
            particles[ i - 1 ],
            particles[ i ],
            m_data.invertColorTime.first );
        }
        else
        {
          LineHelper::updateCustomLineColors(
            line,
            particles[ i - 1 ],
            particles[ i ],
            m_data.lineColor.first,
            m_data.otherLineColor.first,
            m_data.invertColorTime.first );
        }
      }
    }
  }
} // namespace nx
