#pragma once

namespace nx
{
  struct IParticleModifier : public ISerializable< E_ModifierType >
  {
    ~IParticleModifier() override = default;

    virtual void drawMenu() = 0;

    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& modifyParticles(
      const ParticleLayoutData_t& particleLayoutData,
      std::deque< TimedParticle_t >& particles ) = 0;
  };
}