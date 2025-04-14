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
    explicit FreeFallBehavior(const GlobalInfo_t& info)
      : m_globalInfo(info)
    {}

    E_BehaviorType getType() const override { return E_FreeFallBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override
    {}

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override
    {
      const auto trail = p->spawnTime / m_data.timeDivisor;
      p->shape.setPosition( { p->shape.getPosition().x, p->shape.getPosition().y + trail } );
    }

    void drawMenu() override
    {
      ImGui::SliderFloat( "##Free Fall Time", &m_data.timeDivisor, 0.5f, 50.f, "Free Fall Time %0.2f" );
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    FreeFallData_t m_data;
  };
}