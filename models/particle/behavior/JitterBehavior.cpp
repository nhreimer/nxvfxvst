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
                                     const Midi_t& midi,
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