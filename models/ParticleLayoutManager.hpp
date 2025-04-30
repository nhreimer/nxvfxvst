#pragma once

#include "models/particle/SpiralParticleLayout.hpp"

#include "data/PipelineContext.hpp"

namespace nx
{

  class ParticleLayoutManager final
  {
  public:

    explicit ParticleLayoutManager( PipelineContext& context )
      : m_ctx( context ),
        m_particleLayout( std::make_unique< SpiralParticleLayout >( context ) )
    {}

    nlohmann::json serialize() const;
    void deserialize( const nlohmann::json& j ) const;
    void update( const sf::Time& deltaTime ) const;

    void processMidiEvent( const Midi_t& midiEvent ) const;

    const ParticleLayoutData_t& getParticleOptions() const;
    std::deque< TimedParticle_t* >& getParticles() const;

    void drawMenu();

  private:

    // save settings between changes to make editing less frustrating
    template < typename T >
    void changeLayout()
    {
      m_tempSettings[ SerialHelper::serializeEnum( m_particleLayout->getType() ) ] = m_particleLayout->serialize();
      m_particleLayout.reset( new T( m_ctx ) );

      const auto newLayoutName = SerialHelper::serializeEnum( m_particleLayout->getType() );
      if ( m_tempSettings.contains( newLayoutName ) )
        m_particleLayout->deserialize( m_tempSettings[ newLayoutName ] );
    }

  private:
    PipelineContext& m_ctx;

    std::unique_ptr< IParticleLayout > m_particleLayout;

    nlohmann::json m_tempSettings;
  };

}