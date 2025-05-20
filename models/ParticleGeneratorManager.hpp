#pragma once

#include "models/particle/generator/CircleParticleGenerator.hpp"
#include "models/particle/generator/RingGenerator.hpp"

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
        if ( ImGui::RadioButton( "Rings##1", E_ParticleType::E_RingParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new RingParticleGenerator() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    IParticleGenerator * getParticleGenerator() const { return m_particleGenerator.get(); }

  private:

    std::unique_ptr< IParticleGenerator > m_particleGenerator;

  };

}