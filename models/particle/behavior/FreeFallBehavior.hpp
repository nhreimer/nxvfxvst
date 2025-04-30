#pragma once

#include "models/InterfaceTypes.hpp"
#include "models/IParticleBehavior.hpp"

namespace nx
{
  class FreeFallBehavior final : public IParticleBehavior
  {

    struct FreeFallData_t
    {
      float timeDivisor { 2.5f };     // in seconds
    };

  public:
    explicit FreeFallBehavior(PipelineContext& context)
      : m_ctx(context)
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "timeDivisor", m_data.timeDivisor }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      m_data.timeDivisor = j.at( "timeDivisor" ).get<float>();
    }

    E_BehaviorType getType() const override { return E_BehaviorType::E_FreeFallBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override
    {}

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override
    {
      const auto trail = p->spawnTime / m_data.timeDivisor;
      p->shape.setPosition( { p->shape.getPosition().x, p->shape.getPosition().y + trail } );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Free Fall Behavior" ) )
      {
        ImGui::SliderFloat( "##Free Fall Time", &m_data.timeDivisor, 0.5f, 50.f, "Free Fall Time %0.2f" );
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    PipelineContext& m_ctx;
    FreeFallData_t m_data;
  };
}