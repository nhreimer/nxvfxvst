#include "models/particle/behavior/RadialSpreaderBehavior.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json RadialSpreaderBehavior::serialize() const
  {
    return
 {
    { "type", SerialHelper::serializeEnum( getType() ) },
    { "spreadMultiplier", m_data.spreadMultiplier },
    { "speed", m_data.speed }
    };
  }

  void RadialSpreaderBehavior::deserialize(const nlohmann::json &j)
  {
    m_data.spreadMultiplier = j.at( "jitterMultiplier" ).get<float>();
    m_data.speed = j.at( "speed" ).get<float>();
  }

  void RadialSpreaderBehavior::applyOnSpawn( TimedParticle_t * p, const Midi_t& midi )
  {
    sf::Vector2f pos = p->shape.getPosition();
    const sf::Vector2f dir =
      ( pos - m_ctx.globalInfo.windowHalfSize ) * ( m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime );

    // Avoid NaNs if particle spawns directly at center
    if (dir.x == 0.f && dir.y == 0.f) return;

    pos = m_ctx.globalInfo.windowHalfSize + dir * m_data.spreadMultiplier;
    p->shape.setPosition(pos);
  }

  void RadialSpreaderBehavior::applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime )
  {
    const sf::Vector2f baseDir = p->originalPosition - m_ctx.globalInfo.windowHalfSize;

    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;
    const float pulse = std::sin(elapsed * m_data.speed) * 0.5f + 0.5f; // oscillates between [0, 1]

    const sf::Vector2f animatedPos = m_ctx.globalInfo.windowHalfSize + baseDir * (1.f + m_data.spreadMultiplier * pulse);
    p->shape.setPosition(animatedPos);
  }

  void RadialSpreaderBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Radial Spreading Behavior" ) )
    {
      ImGui::SliderFloat( "Spread Multiplier", &m_data.spreadMultiplier, 0.0f, 5.0f );
      ImGui::SliderFloat( "Spread Speed", &m_data.speed, 0.0f, 5.0f );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}