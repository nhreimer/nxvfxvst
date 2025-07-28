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

#include "models/particle/behavior/JitterBehavior.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json JitterBehavior::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(JITTER_BEHAVIOR_PARAMS)
    return j;
  }

  void JitterBehavior::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(JITTER_BEHAVIOR_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void JitterBehavior::applyOnSpawn( IParticle * p,
                                     const ParticleData_t& particleData )
  {
    p->setPosition( getJitterPosition( p ) );
  }

  void JitterBehavior::applyOnUpdate( IParticle * p,
                                      const sf::Time& deltaTime,
                                      const ParticleData_t& particleData )
  {
    p->setPosition( getJitterPosition( p ) );
  }

  void JitterBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Jitter Behavior" ) )
    {
      EXPAND_SHADER_IMGUI(JITTER_BEHAVIOR_PARAMS, m_data)
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  sf::Vector2f JitterBehavior::getJitterPosition( const IParticle * p )
  {
    // add jitter to the position
    // 1. get the circular offset by calculating a random angle [0 - 360)
    const auto jitterAngle = static_cast< float >( m_rand() % 360 ) * NX_D2R;

    // 2. get the deviation amount
    // we have to add 1.f here to prevent the radius from ever being 0 and having a div/0 error.
    auto safeRadius = static_cast< uint32_t >( p->getRadius() );
    if ( safeRadius == 0 ) ++safeRadius;

    const auto jitterAmount = m_data.jitterMultiplier.first *
                                   static_cast< float >( m_rand() % safeRadius );
    return p->getPosition() +
      sf::Vector2f { std::cos( jitterAngle ) * jitterAmount, std::sin( jitterAngle ) * jitterAmount };
  }

}