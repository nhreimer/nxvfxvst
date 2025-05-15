#pragma once

#include "models/particle/generator/CircleParticleGenerator.hpp"
#include "models/particle/generator/StarburstParticleGenerator.hpp"
#include "models/particle/generator/BurstRingGenerator.hpp"

namespace nx
{

  class ParticleGeneratorManager final
  {

  public:

    ParticleGeneratorManager()
      : m_particleGenerator( std::make_unique< CircleParticleGenerator >() )
    {}

    void drawMenu()
    {
      if ( ImGui::TreeNode( "Particles Available" ) )
      {
        if ( ImGui::RadioButton( "Circles##1", E_ParticleType::E_CircleParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new CircleParticleGenerator() );
        if ( ImGui::RadioButton( "Stars##1", E_ParticleType::E_StarburstParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new StarburstParticleGenerator() );
        if ( ImGui::RadioButton( "Burst Ring##1", E_ParticleType::E_BurstRingParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new BurstRingGenerator() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    IParticleGenerator * getParticleGenerator() const { return m_particleGenerator.get(); }

  private:

    std::unique_ptr< IParticleGenerator > m_particleGenerator;

  };

}