#include "models/ParticleLayoutManager.hpp"

#include "models/particle/EmptyParticleLayout.hpp"
#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"

#include "models/particle/LissajousCurveLayout.hpp"
#include "models/particle/FractalRingLayout.hpp"
#include "models/particle/LSystemCurveLayout.hpp"
#include "models/particle/GoldenSpiralLayout.hpp"
#include "models/particle/EllipticalLayout.hpp"

namespace nx
{

  nlohmann::json ParticleLayoutManager::serialize() const
  {
    return m_particleLayout->serialize();
  }

  void ParticleLayoutManager::deserialize( const nlohmann::json& j ) const
  {
    m_particleLayout->deserialize( j );
  }

  void ParticleLayoutManager::update( const sf::Time& deltaTime ) const
  {
    m_particleLayout->update( deltaTime );
  }

  void ParticleLayoutManager::processMidiEvent( const Midi_t& midiEvent ) const
  {
    m_particleLayout->addMidiEvent( midiEvent );
  }

  const ParticleLayoutData_t& ParticleLayoutManager::getParticleOptions() const
  {
    return m_particleLayout->getParticleOptions();
  }

  std::deque< TimedParticle_t* >& ParticleLayoutManager::getParticles() const
  {
    return m_particleLayout->getParticles();
  }

  void ParticleLayoutManager::drawMenu()
  {
    if ( ImGui::TreeNode( "Layouts Available" ) )
    {
      ImGui::Text( "Layouts" );
      {
        if ( ImGui::RadioButton( "Empty", m_particleLayout->getType() == E_LayoutType::E_EmptyLayout ) )
          changeLayout< EmptyParticleLayout >();

        ImGui::SameLine();
        if ( ImGui::RadioButton( "Random", m_particleLayout->getType() == E_LayoutType::E_RandomLayout ) )
          changeLayout< RandomParticleLayout >();

        ImGui::SeparatorText( "Curved Layouts" );

        if ( ImGui::RadioButton( "Spiral", m_particleLayout->getType() == E_LayoutType::E_SpiralLayout ) )
          changeLayout< SpiralParticleLayout >();

        ImGui::SameLine();
        if ( ImGui::RadioButton( "Lissajous Curve", m_particleLayout->getType() == E_LayoutType::E_LissajousCurveLayout ) )
          changeLayout< LissajousCurveLayout >();

        ImGui::SameLine();
        if ( ImGui::RadioButton( "Golden Spiral", m_particleLayout->getType() == E_LayoutType::E_GoldenSpiralLayout ) )
          changeLayout< GoldenSpiralLayout >();

        ImGui::SameLine();
        if ( ImGui::RadioButton( "Elliptical", m_particleLayout->getType() == E_LayoutType::E_EllipticalLayout ) )
          changeLayout< EllipticalLayout >();

        ImGui::SeparatorText( "Fractal Layouts" );

        if ( ImGui::RadioButton( "Fractal Ring", m_particleLayout->getType() == E_LayoutType::E_FractalRingLayout ) )
          changeLayout< FractalRingLayout >();

        ImGui::SameLine();
        if ( ImGui::RadioButton( "L-System Curve", m_particleLayout->getType() == E_LayoutType::E_LSystemCurveLayout ) )
          changeLayout< LSystemCurveLayout >();
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }

    ImGui::Separator();
    m_particleLayout->drawMenu();
  }

}