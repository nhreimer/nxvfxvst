#pragma once

#include "models/IParticle.hpp"
#include "models/ISerializable.hpp"
#include "models/InterfaceTypes.hpp"

#include "data/ParticleData_t.hpp"

namespace nx
{

  struct IParticleBehavior : public ISerializable< E_BehaviorType >
  {
    ~IParticleBehavior() override = default;
    virtual void applyOnSpawn( IParticle * p,
                               const ParticleData_t& particleData ) = 0;

    virtual void applyOnUpdate( IParticle * p,
                                const sf::Time& deltaTime,
                                const ParticleData_t& particleData ) = 0;
    virtual void drawMenu() = 0;
  };

}