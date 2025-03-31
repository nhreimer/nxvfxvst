#pragma once

#include "helpers/ColorHelper.hpp"

#include "models/data/ParticleLayoutData_t.hpp"

namespace nx
{

  /// Provides midi consumption, i.e., adding it to the deque
  /// Provides updates based on timeouts (can be overridden)
  /// must provide setPosition
  class ParticleConsumer : public IParticleLayout
  {
    public:

    explicit ParticleConsumer( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {}

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      const auto noteInfo = MidiHelper::getMidiNote( midiEvent.pitch );
      auto position = getNextPosition( noteInfo );
      position += { static_cast< float >( m_winfo.windowSize.x ) / 2.f,
                    static_cast< float >( m_winfo.windowSize.y ) / 2.f };

      auto& timeParticle = m_particles.emplace_back();
      auto& particle = timeParticle.shape;

      particle.setRadius( m_options.radius );

      timeParticle.initialColor = ColorHelper::getColorPercentage(
        m_options.startColor,
        std::min( midiEvent.velocity + m_options.boostVelocity, 1.f ) );

      particle.setPosition( position );
      particle.setPointCount( m_options.shapeSides );
      particle.setFillColor( timeParticle.initialColor );
      particle.setOutlineThickness( m_options.outlineThickness );
      particle.setOutlineColor( m_options.outlineColor );

      particle.setOrigin( particle.getGlobalBounds().size / 2.f );
    }

    void update( const sf::Time &deltaTime ) override
    {
      for ( auto i = 0; i < m_particles.size(); ++i )
      {
        auto& timeParticle = m_particles[ i ];
        timeParticle.timeLeft += deltaTime.asMilliseconds();
        const auto percentage = static_cast< float >( timeParticle.timeLeft ) /
                           static_cast< float >( m_options.timeoutInMS );

        if ( percentage < 1.f )
        {
          const auto nextColor =
            ColorHelper::getNextColor(
              timeParticle.initialColor,
              m_options.endColor,
              percentage );

          timeParticle.shape.setFillColor( nextColor );
        }
        else
        {
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleOptions() const override { return m_options; }

    [[nodiscard]]
    std::deque< TimedParticle_t > &getParticles() override { return m_particles; }

    protected:

    virtual sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t >& noteInfo ) = 0;

    protected:

      const GlobalInfo_t& m_winfo;
      std::deque< TimedParticle_t > m_particles;

      std::mt19937 m_rand;
      ParticleLayoutData_t m_options;

  };

}