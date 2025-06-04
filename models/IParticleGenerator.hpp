#pragma once

namespace nx
{
  struct IFFTResult;

  // returns the created particle and the bin it came from
  using ParticleCreatedCallback = std::function< void( IParticle *, int32_t ) >;

  struct IParticleGenerator : public ISerializable< E_ParticleType >
  {
    [[nodiscard]]
    virtual const ParticleData_t& getData() const = 0;
    virtual void drawMenu() = 0;

    /// used for creating one not based on a midi event one that just
    /// uses velocity or magnitude or amplitude that goes towards the
    /// initial brightness of the particle. in midi events, that's based on the
    /// velocity. for audio data, it can be based on magnitude or energy or whatever you want
    [[nodiscard]]
    virtual IParticle * createParticle( float velocity, float timeStampInSeconds ) = 0;

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