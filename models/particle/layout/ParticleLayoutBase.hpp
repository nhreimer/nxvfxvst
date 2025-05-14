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
        m_behaviorPipeline( context ),
        m_particleGenerator( std::make_unique< CircleParticleGenerator >() )
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
        IParticle * timeParticle = m_particles[ i ];

        timeParticle->update( deltaTime );

        if ( !timeParticle->hasExpired() )
        {
          const auto percentage = timeParticle->getTimeRemainingPercentage();

          updateFillColor( timeParticle, percentage );
          updateOutlineColor( timeParticle, percentage );

          // notify the behavior pipeline that we've updated a particle
          // this notification occurs automatically but the OnSpawn one does not
          // see notifyBehaviorOnSpawn(...)
          m_behaviorPipeline.applyOnUpdate(
            timeParticle,
            deltaTime,
            m_particleGenerator->getData(),
            m_data.layoutOriginalPosition );
        }
        else
        {
          delete m_particles[ i ];
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleLayoutData() const override { return m_data; }

    [[nodiscard]]
    const ParticleData_t& getParticleData() const override { return m_particleGenerator->getData(); }

    [[nodiscard]]
    std::deque< IParticle * >& getParticles() override { return m_particles; }

  protected:

    /// this is not automatically called because it's uncertain when a particle
    /// and how many particles are created in a layout. each layout is responsible
    /// for calling this
    /// @param timeParticle
    /// @param midiEvent
    virtual void notifyBehaviorOnSpawn( IParticle * timeParticle, const Midi_t& midiEvent )
    {
      m_behaviorPipeline.applyOnSpawn(
        timeParticle,
        midiEvent,
        m_particleGenerator->getData(),
        m_data.layoutOriginalPosition );
    }

    TimeEasing& getEasing() { return m_fadeEasing; }

  private:
    virtual void updateFillColor( IParticle * particle, const float percentage )
    {
      const auto& particleData = m_particleGenerator->getData();

      const auto nextStartColor =
        ColorHelper::getNextColor(
          particleData.fillStartColor,
          sf::Color::Black, // the bg color: which ought to be adjustable
          percentage );

      const auto nextEndColor =
        ColorHelper::getNextColor(
          particleData.fillEndColor,
          sf::Color::Black,
          percentage );

      particle->setColorPattern( nextStartColor, nextEndColor );
    }

    virtual void updateOutlineColor( IParticle * particle, const float percentage )
    {
      const auto& particleData = m_particleGenerator->getData();

      const auto nextStartColor =
        ColorHelper::getNextColor(
          particleData.outlineStartColor,
          sf::Color::Black,
          percentage );

      const auto nextEndColor =
        ColorHelper::getNextColor(
          particleData.outlineEndColor,
          sf::Color::Black,
          percentage );

      particle->setOutlineColorPattern( nextStartColor, nextEndColor );
    }

  protected:
    PipelineContext& m_ctx;
    std::deque< IParticle * > m_particles;

    TParticleData m_data;

    ParticleBehaviorPipeline m_behaviorPipeline;
    std::unique_ptr< IParticleGenerator > m_particleGenerator;

    TimeEasing m_fadeEasing;
  };

}