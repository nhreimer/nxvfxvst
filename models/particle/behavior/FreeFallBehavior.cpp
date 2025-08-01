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

#include "models/particle/behavior/FreeFallBehavior.hpp"

#include "helpers/SerialHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json FreeFallBehavior::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(FREE_FALL_BEHAVIOR_PARAMS)
    return j;
  }

  void FreeFallBehavior::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(FREE_FALL_BEHAVIOR_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void FreeFallBehavior::applyOnUpdate( IParticle * p,
                                        const sf::Time& deltaTime,
                                        const ParticleData_t& particleData )
  {
    const float elapsed = p->getTimeAliveInSeconds();
    const float offset = elapsed * m_data.scrollRate.first;

    p->setPosition({ p->getPosition().x,
                     m_data.invert.first ? p->getPosition().y - offset
                                         : p->getPosition().y + offset });
    // const auto trail = p->getSpawnTimeInSeconds() / m_data.timeDivisor.first;
    // const float offset = trail * deltaTime.asSeconds();
    //
    // if (!m_data.invert.first)
    //   p->move({0.f, offset});     // Downward scroll
    // else
    //   p->move({0.f, -offset});    // Upward scroll
  }

  void FreeFallBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Free Fall Behavior" ) )
    {
      ImGui::SliderFloat( "##Scroll Rate", &m_data.scrollRate.first, 0.f, 50.f, "Scroll Rate %0.2f" );
      ImGui::Checkbox( "Invert", &m_data.invert.first );
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}