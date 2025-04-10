#pragma once

namespace nx
{
  struct IParticleLayout : ISerializable< E_LayoutType >
  {
    ~IParticleLayout() override = default;

    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;
    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual void drawMenu() = 0;

    [[nodiscard]]
    virtual const ParticleLayoutData_t& getParticleOptions() const = 0;

    [[nodiscard]]
    virtual std::deque< TimedParticle_t >& getParticles() = 0;
  };

}