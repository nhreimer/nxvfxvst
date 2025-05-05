#include "models/particle/behavior/MagneticBehavior.hpp"

namespace nx
{

  void MagneticAttractorBehavior::applyOnUpdate(TimedParticle_t * p, const sf::Time& dt)
  {
    const sf::Vector2f pos = p->shape.getPosition();

    const sf::Vector2f attractor { m_data.magnetLocation.first.x * static_cast< float >(m_ctx.globalInfo.windowSize.x ),
                                    m_data.magnetLocation.first.y * static_cast< float >(m_ctx.globalInfo.windowSize.y) };

    const sf::Vector2f dir = attractor - pos;
    const float distance = std::max(length(dir), 0.001f); // avoid divide by 0
    const sf::Vector2f normDir = dir / distance;

    // Falloff based on distance
    float force = m_data.strength.first * 2.f;
    if (m_data.useFalloff.first)
      force *= 1.f / std::pow(distance, m_data.falloffExponent.first);

    // Direction: pull or push
    if (!m_data.isAttracting.first)
      force *= -1.f;

    // Apply movement offset
    const sf::Vector2f offset = normDir * force * dt.asSeconds();
    p->shape.move( offset );
  }

  void MagneticAttractorBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Magnetic Behavior" ) )
    {
      const sf::Vector2f currentPos = m_data.magnetLocation.first;

      EXPAND_SHADER_IMGUI(MAGNET_BEHAVIOR_PARAMS, m_data)

      if ( currentPos != m_data.magnetLocation.first )
      {
        const sf::Vector2f calibrated { m_data.magnetLocation.first.x * static_cast< float >( m_ctx.globalInfo.windowSize.x ),
                                        m_data.magnetLocation.first.y * static_cast< float >( m_ctx.globalInfo.windowSize.y ) };
        m_timedCursor.setPosition( calibrated );
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursor.hasExpired() )
      m_timedCursor.drawPosition();
  }

  nlohmann::json MagneticAttractorBehavior::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(MAGNET_BEHAVIOR_PARAMS)
    return j;
  }

  void MagneticAttractorBehavior::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(MAGNET_BEHAVIOR_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }


} // namespace nx
