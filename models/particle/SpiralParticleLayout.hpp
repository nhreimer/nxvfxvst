#pragma once

#include <random>

#include "helpers/MidiHelper.hpp"
#include "helpers/MathHelper.hpp"

#include "models/particle/ParticleConsumer.hpp"

#include "helpers/MenuHelper.hpp"


namespace nx
{

  class SpiralParticleLayout final : public ParticleConsumer
  {
  public:

    explicit SpiralParticleLayout( const WindowInfo_t& winfo )
      : ParticleConsumer( winfo )
    {}

    ~SpiralParticleLayout() override = default;

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Particle Layout " ) )
      {
        drawAppearanceMenu();
        drawAdjustmentMenu();
        drawPositionMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  protected:


    sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t >& noteInfo ) override
    {
      const auto noteNumber = std::get< 0 >( noteInfo );
      const auto noteOctave = std::get< 1 >( noteInfo );

      // 0. calculate the position based on the note, octave, and spread
      auto position = MathHelper::getAnglePosition( 12,
                                                      noteNumber,
                                                      static_cast< float >( noteOctave ) * m_options.spreadMultiplier,
                                                      static_cast< float >( noteOctave ) * m_options.spreadMultiplier );

      // add jitter to the position
      // 1. get the circular offset by calculating a random angle [0 - 360)
      const auto jitterAngle = static_cast< float >( m_rand() % 360 ) * POLY_D2R;

      // 2. get the deviation amount
      // we have to add 1.f here to prevent the radius from ever being 0 and having a div/0 error.
      auto safeRadius = static_cast< uint32_t >( m_options.radius );
      if ( safeRadius == 0 ) ++safeRadius;

      const auto jitterAmount = m_options.jitterMultiplier *
                                     static_cast< float >( m_rand() % safeRadius );

      position += { std::cos( jitterAngle ) * jitterAmount, std::sin( jitterAngle ) * jitterAmount };

      // use sf::View to create the center, but don't individually offset all by the center
      // add the x, y offsets
      // position += ( center + options.positionOffset );
      position += m_options.positionOffset;

      return position;
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

        ImGui::Separator();
        ImGui::SliderFloat( "Thickness##2", &m_options.outlineThickness, 0.f, 25.f );

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
        ImGui::SliderFloat( "Spread##1", &m_options.spreadMultiplier, 0.f, 5.f );
        ImGui::SliderFloat( "Jitter##1", &m_options.jitterMultiplier, 0.f, 10.f );
        ImGui::SliderFloat( "Boost##1", &m_options.boostVelocity, 0.f, 1.f );

        MenuHelper::drawBlendOptions( m_options.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void drawPositionMenu()
    {
      if ( ImGui::TreeNode( "Particle Position" ) )
      {
        const sf::Vector2f halfWindow =
        {
          static_cast< float >( m_winfo.windowSize.x ) / 2.f,
          static_cast< float >( m_winfo.windowSize.y ) / 2.f
        };

        float xOffset = m_options.positionOffset.x;
        if ( ImGui::SliderFloat( "x-axis##1", &xOffset, -halfWindow.x, halfWindow.x ) )
          m_options.positionOffset.x = xOffset;

        float yOffset = m_options.positionOffset.y;
        if ( ImGui::SliderFloat( "y-axis##1", &yOffset, -halfWindow.y, halfWindow.y ) )
          m_options.positionOffset.y = yOffset;

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }


  private:

    std::mt19937 m_rand;

  };

}