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
    const auto trail = p->getSpawnTimeInSeconds() / m_data.timeDivisor.first;
    p->setPosition( { p->getPosition().x, p->getPosition().y + trail } );
  }

  void FreeFallBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Free Fall Behavior" ) )
    {
      ImGui::SliderFloat( "##Free Fall Time", &m_data.timeDivisor.first, 0.5f, 50.f, "Free Fall Time %0.2f" );
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}