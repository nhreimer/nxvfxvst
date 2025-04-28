#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class RadialSpreaderBehavior final : public IParticleBehavior
  {

    struct RadialSpreaderData_t
    {
      float spreadMultiplier = 1.5f;
      float speed = 0.5f;
    };

  public:
    explicit RadialSpreaderBehavior(PipelineContext& context)
      : m_ctx( context )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
   {
      { "type", SerialHelper::serializeEnum( getType() ) },
      { "spreadMultiplier", m_data.spreadMultiplier },
      { "speed", m_data.speed }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      m_data.spreadMultiplier = j.at( "jitterMultiplier" ).get<float>();
      m_data.speed = j.at( "speed" ).get<float>();
    }

    E_BehaviorType getType() const override { return E_BehaviorType::E_RadialSpreaderBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override
    {
      sf::Vector2f pos = p->shape.getPosition();
      const sf::Vector2f dir =
        ( pos - m_ctx.globalInfo.windowHalfSize ) * ( m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime );

      // Avoid NaNs if particle spawns directly at center
      if (dir.x == 0.f && dir.y == 0.f) return;

      pos = m_ctx.globalInfo.windowHalfSize + dir * m_data.spreadMultiplier;
      p->shape.setPosition(pos);
    }

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override
    {
      const sf::Vector2f baseDir = p->originalPosition - m_ctx.globalInfo.windowHalfSize;

      const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;
      const float pulse = std::sin(elapsed * m_data.speed) * 0.5f + 0.5f; // oscillates between [0, 1]

      const sf::Vector2f animatedPos = m_ctx.globalInfo.windowHalfSize + baseDir * (1.f + m_data.spreadMultiplier * pulse);
      p->shape.setPosition(animatedPos);
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Radial Spreading Behavior" ) )
      {
        ImGui::SliderFloat( "Spread Multiplier", &m_data.spreadMultiplier, 0.0f, 5.0f );
        ImGui::SliderFloat( "Spread Speed", &m_data.speed, 0.0f, 5.0f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    PipelineContext& m_ctx;
    RadialSpreaderData_t m_data;
  };
}