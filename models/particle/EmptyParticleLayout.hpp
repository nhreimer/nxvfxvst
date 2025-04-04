#pragma once

#include "imgui.h"

#include "models/Interfaces.hpp"

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class EmptyParticleLayout final : public ParticleConsumer
  {
  public:
    explicit EmptyParticleLayout( const GlobalInfo_t& globalInfo )
      : ParticleConsumer( globalInfo )
    {}

    ~EmptyParticleLayout() override = default;

    E_LayoutType getType() const override { return E_EmptyLayout; }

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Particle Layout" ) )
      {
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void addMidiEvent( const Midi_t &midiEvent ) override {}
    void update( const sf::Time &deltaTime ) override {}

  protected:

    sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t > &noteInfo ) override
    {
      return { 0.f, 0.f };
    }

  private:


  };

}