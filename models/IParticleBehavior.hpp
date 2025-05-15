#pragma once

#include "models/InterfaceTypes.hpp"

namespace nx
{

  struct IParticleBehavior : public ISerializable< E_BehaviorType >
  {
    ~IParticleBehavior() override = default;
    virtual void applyOnSpawn( IParticle * p,
                               const Midi_t& midi,
                               const ParticleData_t& particleData ) = 0;

    virtual void applyOnUpdate( IParticle * p,
                                const sf::Time& deltaTime,
                                const ParticleData_t& particleData ) = 0;
    virtual void drawMenu() = 0;
  };

}