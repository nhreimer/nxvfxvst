#include "models/particle/behavior/JitterBehavior.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json JitterBehavior::serialize() const
  {
    return
 {
      { "type", SerialHelper::serializeEnum( getType() ) },
      { "jitterMultiplier", m_data.jitterMultiplier }
    };
  }

  void JitterBehavior::deserialize(const nlohmann::json &j)
  {
    m_data.jitterMultiplier = j.at( "jitterMultiplier" ).get<float>();
  }

  void JitterBehavior::applyOnSpawn( TimedParticle_t * p, const Midi_t& midi )
  {
    p->shape.setPosition( getJitterPosition( p ) );
  }

  void JitterBehavior::applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime )
  {
    p->shape.setPosition( getJitterPosition( p ) );
  }

  void JitterBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Jitter Behavior" ) )
    {
      ImGui::SliderFloat( "Jitter", &m_data.jitterMultiplier, 0.f, 5.f );
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  sf::Vector2f JitterBehavior::getJitterPosition( const TimedParticle_t * p )
  {
    // add jitter to the position
    // 1. get the circular offset by calculating a random angle [0 - 360)
    const auto jitterAngle = static_cast< float >( m_rand() % 360 ) * NX_D2R;

    // 2. get the deviation amount
    // we have to add 1.f here to prevent the radius from ever being 0 and having a div/0 error.
    auto safeRadius = static_cast< uint32_t >( p->shape.getRadius() );
    if ( safeRadius == 0 ) ++safeRadius;

    const auto jitterAmount = m_data.jitterMultiplier *
                                   static_cast< float >( m_rand() % safeRadius );
    return p->shape.getPosition() +
      sf::Vector2f { std::cos( jitterAngle ) * jitterAmount, std::sin( jitterAngle ) * jitterAmount };
  }

}