#pragma once

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

class RandomParticleLayout final : public ParticleConsumer< ParticleLayoutData_t >
{
public:

  explicit RandomParticleLayout( const GlobalInfo_t& winfo )
  : ParticleConsumer( winfo )
  {}

  E_LayoutType getType() const override { return E_LayoutType::E_RandomLayout; }

  void drawMenu() override
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Random Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

protected:
  sf::Vector2f getNextPosition( const Midi_t & midiNote ) override
  {
    return { static_cast< float >( m_rand() % m_globalInfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_globalInfo.windowSize.y ) };
  }

private:

  std::mt19937 m_rand;

};

}