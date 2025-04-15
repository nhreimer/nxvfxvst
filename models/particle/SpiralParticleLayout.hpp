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
    E_LayoutType getType() const override { return E_LayoutType::E_SpiralLayout; }

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Particle Layout " ) )
      {
        ParticleHelper::drawMenu( m_data );
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
      return MathHelper::getAnglePosition( 12,
                                                      noteNumber,
                                                      static_cast< float >( noteOctave ),
                                                      static_cast< float >( noteOctave ) );
    }

  private:

    std::mt19937 m_rand;

    TimedCursorPosition m_timedDrawing;

  };

}