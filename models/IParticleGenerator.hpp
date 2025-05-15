#pragma once

namespace nx
{
  struct IParticleGenerator : public ISerializable< E_ParticleType >
  {
    [[nodiscard]]
    virtual const ParticleData_t& getData() const = 0;
    virtual void drawMenu() = 0;

    [[nodiscard]]
    virtual IParticle * createParticle( const Midi_t& midiEvent,
                                        float timeStampInSeconds ) = 0;

    // same as the one above but allows us to set the radius
    // in a layout needs to make any adjustment
    [[nodiscard]]
    virtual IParticle * createParticle( const Midi_t& midiEvent,
                                        float timeStampInSeconds,
                                        float radius ) = 0;
  };
}