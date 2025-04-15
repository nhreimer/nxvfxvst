#pragma once

#include <random>

#include "helpers/CommonHeaders.hpp"

#include "helpers/MidiHelper.hpp"
#include "helpers/ColorHelper.hpp"

#include "models/data/ParticleLayoutData_t.hpp"

#include "models/ParticleBehaviorPipeline.hpp"

namespace nx
{

  /// Provides midi consumption, i.e., adding it to the deque
  /// Provides updates based on timeouts (can be overridden)
  /// must provide setPosition
  template< typename TParticleData_t >
  class ParticleConsumer : public IParticleLayout
  {

    static_assert( std::is_base_of_v< ParticleLayoutData_t, TParticleData_t >, "Invalid inherited template parameter" );

    public:

    explicit ParticleConsumer(const GlobalInfo_t &winfo)
        : m_globalInfo( winfo ),
          m_behaviorPipeline( winfo )
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
      auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
      j[ "behaviors" ] = m_behaviorPipeline.saveModifierPipeline();
      return j;
    }

    void deserialize(const nlohmann::json& j) override
    {
      ParticleHelper::deserialize( m_data, j );
      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadModifierPipeline( j.at( "behaviors" ) );
    }

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      auto position = getNextPosition( midiEvent );
      position += m_globalInfo.windowHalfSize;

      auto * timeParticle = m_particles.emplace_back( new TimedParticle_t() );
      timeParticle->originalPosition = position;
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
      timeParticle->spawnTime = m_globalInfo.elapsedTimeSeconds;

      m_behaviorPipeline.applyOnSpawn( timeParticle, midiEvent );
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
          m_behaviorPipeline.applyOnUpdate( timeParticle, deltaTime );
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

    //virtual sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t >& noteInfo ) = 0;
    virtual sf::Vector2f getNextPosition( const Midi_t& midi ) = 0;

    protected:

      const GlobalInfo_t &m_globalInfo;
      std::deque< TimedParticle_t * > m_particles;

      std::mt19937 m_rand;
      TParticleData_t m_data;

      ParticleBehaviorPipeline m_behaviorPipeline;
  };

}