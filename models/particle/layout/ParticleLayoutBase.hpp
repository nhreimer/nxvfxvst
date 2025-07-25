#pragma once

#include "helpers/ColorHelper.hpp"

#include "models/ParticleBehaviorPipeline.hpp"
#include "models/easings/PercentageEasing.hpp"

#include "models/ParticleGeneratorManager.hpp"

namespace nx
{

  ///
  /// @tparam TParticleData inherits from ParticleLayoutData_t
  /// contains the basic functions necessary to easily get a particle layout going
  template < typename TParticleData >
  class ParticleLayoutBase : public IParticleLayout
  {

  public:

    explicit ParticleLayoutBase( PipelineContext& context )
      : m_ctx( context ),
        m_behaviorPipeline( context ),
        m_particleGeneratorManager( context )
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
          const auto percentage = timeParticle->getTimeRemainingPercentage(); //m_fadeEasing.getEasing( 1.f - timeParticle->getTimeRemainingPercentage() );

          updateFillColor( timeParticle, percentage );
          updateOutlineColor( timeParticle, percentage );

          // notify the behavior pipeline that we've updated a particle
          // this notification occurs automatically but the OnSpawn one does not
          // see notifyBehaviorOnSpawn(...)
          m_behaviorPipeline.applyOnUpdate(
            timeParticle,
            deltaTime,
            m_particleGeneratorManager.getParticleGenerator()->getData() );
        }
        else
        {
          delete m_particles[ i ];
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleData_t& getParticleData() const override
    {
      return m_particleGeneratorManager.getParticleGenerator()->getData();
    }

    [[nodiscard]]
    std::deque< IParticle * >& getParticles() override { return m_particles; }

  protected:

    /// this is not automatically called because it's uncertain when a particle
    /// and how many particles are created in a layout. each layout is responsible
    /// for calling this
    /// @param timeParticle
    virtual void notifyBehaviorOnSpawn( IParticle * timeParticle )
    {

      m_behaviorPipeline.applyOnSpawn(
        timeParticle,
        m_particleGeneratorManager.getParticleGenerator()->getData() );
    }

    PercentageEasing& getEasing() { return m_fadeEasing; }

  private:
    virtual void updateFillColor( IParticle * particle, const float percentage )
    {
      const auto& particleData =
        m_particleGeneratorManager.getParticleGenerator()->getData();

      const auto nextStartColor =
        ColorHelper::getNextColor(
          particleData.fillStartColor.first,
          sf::Color::Black, // the bg color: which ought to be adjustable
          percentage );

      const auto nextEndColor =
        ColorHelper::getNextColor(
          particleData.fillEndColor.first,
          sf::Color::Black,
          percentage );

      particle->setColorPattern( nextStartColor, nextEndColor );
    }

    virtual void updateOutlineColor( IParticle * particle, const float percentage )
    {
      const auto& particleData =
        m_particleGeneratorManager.getParticleGenerator()->getData();

      const auto nextStartColor =
        ColorHelper::getNextColor(
          particleData.outlineStartColor.first,
          sf::Color::Black,
          percentage );

      const auto nextEndColor =
        ColorHelper::getNextColor(
          particleData.outlineEndColor.first,
          sf::Color::Black,
          percentage );

      particle->setOutlineColorPattern( nextStartColor, nextEndColor );
    }

  protected:
    PipelineContext& m_ctx;
    std::deque< IParticle * > m_particles;

    TParticleData m_data;

    ParticleBehaviorPipeline m_behaviorPipeline;
    ParticleGeneratorManager m_particleGeneratorManager;

    PercentageEasing m_fadeEasing;

    sf::BlendMode m_blendMode { sf::BlendAdd };
  };

}