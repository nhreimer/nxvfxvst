#pragma once

#include "models/particle/behavior/FreeFallBehavior.hpp"

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