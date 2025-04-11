#pragma once

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

class RandomParticleLayout final : public ParticleConsumer
{
public:

  explicit RandomParticleLayout( const GlobalInfo_t& winfo )
  : ParticleConsumer( winfo )
  {}

  E_LayoutType getType() const override { return E_RandomLayout; }

  void drawMenu() override
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Particle Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

protected:
  sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t > &noteInfo ) override
  {
    return { static_cast< float >( m_rand() % m_globalInfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_globalInfo.windowSize.y ) };
  }

private:

  std::mt19937 m_rand;

};

}