#pragma once

#include "helpers/ColorHelper.hpp"

#include "models/ParticleBehaviorPipeline.hpp"

namespace nx
{

  ///
  /// @tparam TParticleData inherits from ParticleLayoutData_t
  /// contains the basic functions necessary to easily get a particle layout going
  template < typename TParticleData >
  class ParticleLayoutBase : public IParticleLayout
  {
    static_assert( std::is_base_of_v< ParticleLayoutData_t, TParticleData >, "Invalid inherited template parameter" );

  public:

    explicit ParticleLayoutBase( PipelineContext& context )
      : m_ctx( context ),
        m_behaviorPipeline( context )
    {}

    ~ParticleLayoutBase() override
    {
      for ( const auto * particle : m_particles )
        delete particle;
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
    /// the position SHOULD be set on the shape before calling this
    /// applies the basic ParticleLayoutData_t options
    /// and calls the behavior application
    virtual void initializeParticle( TimedParticle_t * timeParticle, const Midi_t& midiEvent )
    {
      timeParticle->originalPosition = timeParticle->shape.getPosition();
      auto& particle = timeParticle->shape;

      particle.setRadius( m_data.radius +
                          m_data.velocitySizeMultiplier * midiEvent.velocity );

      timeParticle->initialColor = ColorHelper::getColorPercentage(
        m_data.startColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      particle.setPointCount( m_data.shapeSides );
      particle.setFillColor( timeParticle->initialColor );
      particle.setOutlineThickness( m_data.outlineThickness );
      particle.setOutlineColor( m_data.outlineColor );

      particle.setOrigin( particle.getGlobalBounds().size / 2.f );

      // timestamp it
      timeParticle->spawnTime = m_ctx.globalInfo.elapsedTimeSeconds;

      m_behaviorPipeline.applyOnSpawn( timeParticle, midiEvent );
    }

  protected:
    PipelineContext& m_ctx;
    std::deque< TimedParticle_t * > m_particles;

    TParticleData m_data;

    ParticleBehaviorPipeline m_behaviorPipeline;
  };

}