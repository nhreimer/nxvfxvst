#include "models/particle/behavior/RadialSpreaderBehavior.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json RadialSpreaderBehavior::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(RADIAL_SPREADER_BEHAVIOR_PARAMS)
    return j;
  }

  void RadialSpreaderBehavior::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(RADIAL_SPREADER_BEHAVIOR_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void RadialSpreaderBehavior::applyOnSpawn( TimedParticle_t * p, const Midi_t& midi )
  {
    sf::Vector2f pos = p->shape.getPosition();
    const sf::Vector2f dir =
      ( pos - m_ctx.globalInfo.windowHalfSize ) * ( m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime );

    // Avoid NaNs if particle spawns directly at center
    if (dir.x == 0.f && dir.y == 0.f) return;

    pos = m_ctx.globalInfo.windowHalfSize + dir * m_data.spreadMultiplier.first;
    p->shape.setPosition(pos);
  }

  void RadialSpreaderBehavior::applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime )
  {
    const sf::Vector2f baseDir = p->originalPosition - m_ctx.globalInfo.windowHalfSize;

    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;
    const float pulse = std::sin(elapsed * m_data.speed.first) * 0.5f + 0.5f; // oscillates between [0, 1]

    const sf::Vector2f animatedPos = m_ctx.globalInfo.windowHalfSize + baseDir * (1.f + m_data.spreadMultiplier.first * pulse);
    p->shape.setPosition(animatedPos);
  }

  void RadialSpreaderBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Radial Spreading Behavior" ) )
    {
      EXPAND_SHADER_IMGUI(RADIAL_SPREADER_BEHAVIOR_PARAMS, m_data)

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}