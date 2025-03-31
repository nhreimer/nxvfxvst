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

  void drawMenu() override
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Particle Layout " ) )
    {
      drawAppearanceMenu();
      drawAdjustmentMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

protected:
  sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t > &noteInfo ) override
  {
    return { static_cast< float >( m_rand() % m_winfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_winfo.windowSize.y ) };
  }

private:

  void drawAppearanceMenu()
  {
    if ( ImGui::TreeNode( "Particle Appearance" ) )
    {
      ImVec4 color = m_options.startColor;

      if ( ImGui::ColorPicker4( "Particle Fill##1",
                                reinterpret_cast< float * >( &color ),
                                ImGuiColorEditFlags_AlphaBar,
                                nullptr ) )
      {
        m_options.startColor = color;
      }

      ImVec4 outlineColor = m_options.outlineColor;

      if ( ImGui::ColorPicker4( "Particle Outline##1",
                                reinterpret_cast< float * >( &outlineColor ),
                                ImGuiColorEditFlags_AlphaBar,
                                nullptr ) )
      {
        m_options.outlineColor = outlineColor;
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void drawAdjustmentMenu()
  {
    if ( ImGui::TreeNode( "Particle Adjust" ) )
    {
      int32_t sides = m_options.shapeSides;
      if ( ImGui::SliderInt( "Sides##1", &sides, 3, 30 ) ) m_options.shapeSides = sides;
      ImGui::SliderFloat( "Radius##1", &m_options.radius, 1.0f, 100.0f );
      ImGui::SliderInt( "Timeout##1", &m_options.timeoutInMS, 15, 10000 );
      // ImGui::SliderFloat( "Spread##1", &m_options.spreadMultiplier, 0.f, 5.f );
      // ImGui::SliderFloat( "Jitter##1", &m_options.jitterMultiplier, 0.f, 10.f );
      ImGui::SliderFloat( "Boost##1", &m_options.boostVelocity, 0.f, 1.f );
      MenuHelper::drawBlendOptions( m_options.blendMode );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

private:

  std::mt19937 m_rand;

};

}