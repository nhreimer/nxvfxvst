#pragma once

namespace nx
{

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class EmptyParticleLayout final : public IParticleLayout
  {
  public:
    explicit EmptyParticleLayout( const GlobalInfo_t& globalInfo ) {}
    ~EmptyParticleLayout() override = default;
    void drawMenu() override {}
    void addMidiEvent( const Midi_t &midiEvent ) override {}
    void update( const sf::Time &deltaTime ) override {}

    [[nodiscard]]
    const ParticleLayoutData_t &getParticleOptions() const override
    {
      return m_emptyParticleData;
    }

    [[nodiscard]]
    std::deque< TimedParticle_t > &getParticles() override
    {
      return m_emptyParticles;
    }

  private:

    std::deque< TimedParticle_t > m_emptyParticles;
    ParticleLayoutData_t m_emptyParticleData;

  };

}