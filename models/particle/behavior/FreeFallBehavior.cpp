#pragma once

#include "models/particle/behavior/FreeFallBehavior.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json FreeFallBehavior::serialize() const
  {
    return
    {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "timeDivisor", m_data.timeDivisor }
    };
  }

  void FreeFallBehavior::deserialize(const nlohmann::json &j)
  {
    m_data.timeDivisor = j.at( "timeDivisor" ).get<float>();
  }

  void FreeFallBehavior::applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime )
  {
    const auto trail = p->spawnTime / m_data.timeDivisor;
    p->shape.setPosition( { p->shape.getPosition().x, p->shape.getPosition().y + trail } );
  }

  void FreeFallBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Free Fall Behavior" ) )
    {
      ImGui::SliderFloat( "##Free Fall Time", &m_data.timeDivisor, 0.5f, 50.f, "Free Fall Time %0.2f" );
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}