#pragma once

#include "models/InterfaceTypes.hpp"

namespace nx
{

  struct IParticleBehavior
  {
    virtual ~IParticleBehavior() = default;
    virtual void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) = 0;
    virtual void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) = 0;
    virtual void drawMenu() = 0;
    virtual E_BehaviorType getType() const = 0;
  };

}