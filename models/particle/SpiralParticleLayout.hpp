#pragma once

#include <random>

#include "helpers/MathHelper.hpp"
#include "helpers/MenuHelper.hpp"

#include "models/particle/ParticleConsumer.hpp"

#include "shapes/TimedCursorPosition.hpp"

namespace nx
{

  class SpiralParticleLayout final : public ParticleConsumer< ParticleLayoutData_t >
  {
  public:

    explicit SpiralParticleLayout( const GlobalInfo_t& winfo )
      : ParticleConsumer( winfo )
    {}

    ~SpiralParticleLayout() override = default;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_SpiralLayout; }

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Particle Layout " ) )
      {
        ParticleHelper::drawMenu( m_data );
        drawPositionMenu();

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedDrawing.hasExpired() )
        m_timedDrawing.drawPosition();
    }

  protected:


    sf::Vector2f getNextPosition( const Midi_t& midiNote ) override
    {
      const auto noteInfo = MidiHelper::getMidiNote( midiNote.pitch );

      const auto noteNumber = std::get< 0 >( noteInfo );
      const auto noteOctave = std::get< 1 >( noteInfo );

      // 0. calculate the position based on the note, octave, and spread
      auto position = MathHelper::getAnglePosition( 12,
                                                      noteNumber,
                                                      static_cast< float >( noteOctave ) * m_data.spreadMultiplier,
                                                      static_cast< float >( noteOctave ) * m_data.spreadMultiplier );

      // add jitter to the position
      // 1. get the circular offset by calculating a random angle [0 - 360)
      const auto jitterAngle = static_cast< float >( m_rand() % 360 ) * NX_D2R;

      // 2. get the deviation amount
      // we have to add 1.f here to prevent the radius from ever being 0 and having a div/0 error.
      auto safeRadius = static_cast< uint32_t >( m_data.radius );
      if ( safeRadius == 0 ) ++safeRadius;

      const auto jitterAmount = m_data.jitterMultiplier *
                                     static_cast< float >( m_rand() % safeRadius );

      position += { std::cos( jitterAngle ) * jitterAmount, std::sin( jitterAngle ) * jitterAmount };

      // use sf::View to create the center, but don't individually offset all by the center
      // add the x, y offsets
      // position += ( center + options.positionOffset );
      position += m_data.positionOffset;

      return position;
    }

  private:

    void drawPositionMenu()
    {
      if ( ImGui::TreeNode( "Particle Position" ) )
      {
        const sf::Vector2f halfWindow =
        {
          static_cast< float >( m_globalInfo.windowSize.x ) / 2.f,
          static_cast< float >( m_globalInfo.windowSize.y ) / 2.f
        };

        float xOffset = m_data.positionOffset.x;
        if ( ImGui::SliderFloat( "x-axis##1", &xOffset, -halfWindow.x, halfWindow.x ) )
        {
          m_data.positionOffset.x = xOffset;
          m_timedDrawing.setPosition( { halfWindow.x + xOffset, halfWindow.y + m_data.positionOffset.y } );
        }
        float yOffset = m_data.positionOffset.y;
        if ( ImGui::SliderFloat( "y-axis##1", &yOffset, -halfWindow.y, halfWindow.y ) )
        {
          m_data.positionOffset.y = yOffset;
          m_timedDrawing.setPosition( { halfWindow.x + m_data.positionOffset.x, halfWindow.y + yOffset } );
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }


  private:

    std::mt19937 m_rand;

    TimedCursorPosition m_timedDrawing;

  };

}