#include "models/particle/behavior/MagneticBehavior.hpp"

namespace nx
{

  void MagneticAttractorBehavior::applyOnUpdate(TimedParticle_t * p, const sf::Time& dt)
  {
    const sf::Vector2f pos = p->shape.getPosition();

    const sf::Vector2f attractor { m_data.magnetLocation.x * static_cast< float >(m_ctx.globalInfo.windowSize.x ),
                                    m_data.magnetLocation.y * static_cast< float >(m_ctx.globalInfo.windowSize.y) };

    const sf::Vector2f dir = attractor - pos;
    const float distance = std::max(length(dir), 0.001f); // avoid divide by 0
    const sf::Vector2f normDir = dir / distance;

    // Falloff based on distance
    float force = m_data.strength * 2.f;
    if (m_data.useFalloff)
      force *= 1.f / std::pow(distance, m_data.falloffExponent);

    // Direction: pull or push
    if (!m_data.isAttracting)
      force *= -1.f;

    // Apply movement offset
    const sf::Vector2f offset = normDir * force * dt.asSeconds();
    p->shape.move( offset );
  }

  void MagneticAttractorBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Magnetic Behavior" ) )
    {
      ImGui::Checkbox("Attracting", &m_data.isAttracting);
      ImGui::SliderFloat("Strength", &m_data.strength, 0.0f, 500.0f);
      ImGui::Checkbox("Use Falloff", &m_data.useFalloff);
      ImGui::SliderFloat("Falloff Exponent", &m_data.falloffExponent, 0.1f, 4.0f);
      // ImGui::Checkbox("Follow Mouse", &m_followMouse);

      if ( ImGui::SliderFloat( "Magnet x##1", &m_data.magnetLocation.x, 0.f, 1.f ) ||
           ImGui::SliderFloat( "Magnet y##1", &m_data.magnetLocation.y, 0.f, 1.f ) )
      {
        const sf::Vector2f calibrated { m_data.magnetLocation.x * static_cast< float >( m_ctx.globalInfo.windowSize.x ),
                                        m_data.magnetLocation.y * static_cast< float >( m_ctx.globalInfo.windowSize.y ) };
        m_timedCursor.setPosition( calibrated );
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursor.hasExpired() )
      m_timedCursor.drawPosition();
  }

}