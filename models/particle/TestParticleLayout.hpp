#pragma once

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

  struct TestParticleLayoutData_t : public ParticleLayoutData_t
  {

  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class TestParticleLayout final : public ParticleConsumer< TestParticleLayoutData_t >
  {
  public:
    explicit TestParticleLayout( const GlobalInfo_t& globalInfo )
      : ParticleConsumer( globalInfo )
    {}

    ~TestParticleLayout() override = default;

    E_LayoutType getType() const override { return E_TestLayout; }

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Test Particle Layout" ) )
      {
        ParticleHelper::drawMenu( m_data );

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      ParticleConsumer::addMidiEvent( midiEvent );

    }

    void update( const sf::Time &deltaTime ) override
    {
      ParticleConsumer::update( deltaTime );

    }

  protected:

    sf::Vector2f getNextPosition( const Midi_t & midiNote ) override
    {
      return { 0.f, 0.f };
    }

  private:


  };

}