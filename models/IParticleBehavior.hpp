#pragma once

#include "models/InterfaceTypes.hpp"

namespace nx
{

  struct IParticleBehavior : public ISerializable< E_BehaviorType >
  {
    ~IParticleBehavior() override = default;
    virtual void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) = 0;
    virtual void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) = 0;
    virtual void drawMenu() = 0;
  };

}