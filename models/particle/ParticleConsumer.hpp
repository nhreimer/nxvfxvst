#pragma once

#include <random>

#include "helpers/CommonHeaders.hpp"

#include "helpers/MidiHelper.hpp"
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

    explicit ParticleConsumer(const GlobalInfo_t &winfo)
        : m_globalInfo( winfo )
      {}

    ~ParticleConsumer() override
    {
      for ( const auto * particle : m_particles )
        delete particle;
    }

      ///////////////////////////////////////////////////////
      /// ISERIALIZABLE
      ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    }

    void deserialize(const nlohmann::json& j) override
    {
      ParticleHelper::deserialize( m_data, j );
    }

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      const auto noteInfo = MidiHelper::getMidiNote( midiEvent.pitch );
      auto position = getNextPosition( noteInfo );
      position += { static_cast< float >( m_globalInfo.windowSize.x ) / 2.f,
                    static_cast< float >( m_globalInfo.windowSize.y ) / 2.f };

      auto * timeParticle = m_particles.emplace_back( new TimedParticle_t() );
      auto& particle = timeParticle->shape;

      particle.setRadius( m_data.radius +
                          m_data.velocitySizeMultiplier * midiEvent.velocity );

      timeParticle->initialColor = ColorHelper::getColorPercentage(
        m_data.startColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      particle.setPosition( position );
      particle.setPointCount( m_data.shapeSides );
      particle.setFillColor( timeParticle->initialColor );
      particle.setOutlineThickness( m_data.outlineThickness );
      particle.setOutlineColor( m_data.outlineColor );

      particle.setOrigin( particle.getGlobalBounds().size / 2.f );

      // timestamp it
      timeParticle->spawnTime = m_clock.getElapsedTime().asSeconds();
    }

    void update( const sf::Time &deltaTime ) override
    {
      for ( auto i = 0; i < m_particles.size(); ++i )
      {
        const auto& timeParticle = m_particles[ i ];
        timeParticle->timeLeft += deltaTime.asMilliseconds();
        const auto percentage = static_cast< float >( timeParticle->timeLeft ) /
                           static_cast< float >( m_data.timeoutInMS );

        if ( percentage < 1.f )
        {
          const auto nextColor =
            ColorHelper::getNextColor(
              timeParticle->initialColor,
              m_data.endColor,
              percentage );

          timeParticle->shape.setFillColor( nextColor );
        }
        else
        {
          delete m_particles[ i ];
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleOptions() const override { return m_data; }

    [[nodiscard]]
    std::deque< TimedParticle_t * > &getParticles() override { return m_particles; }

    protected:

    virtual sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t >& noteInfo ) = 0;

    protected:

      const GlobalInfo_t &m_globalInfo;
      std::deque< TimedParticle_t * > m_particles;

      std::mt19937 m_rand;
      ParticleLayoutData_t m_data;

      sf::Clock m_clock;

  };

}