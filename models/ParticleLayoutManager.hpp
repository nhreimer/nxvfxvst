#pragma once

#include "models/particle/EmptyParticleLayout.hpp"
#include "models/particle/SpiralParticleLayout.hpp"
#include "models/particle/RandomParticleLayout.hpp"

#include "models/particle/LissajousCurveLayout.hpp"
#include "models/particle/FractalRingLayout.hpp"
#include "models/particle/LSystemCurveLayout.hpp"
#include "models/particle/GoldenSpiralLayout.hpp"
#include "models/particle/EllipticalLayout.hpp"
#include "models/particle/TestParticleLayout.hpp"

namespace nx
{

  class ParticleLayoutManager final
  {
  public:

    explicit ParticleLayoutManager( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo ),
        m_particleLayout( std::make_unique< SpiralParticleLayout >( globalInfo ) )
    {}

    nlohmann::json serialize() const
    {
      return m_particleLayout->serialize();
    }

    void deserialize( const nlohmann::json& j ) const
    {
      m_particleLayout->deserialize( j );
    }

    void update( const sf::Time& deltaTime ) const
    {
      m_particleLayout->update( deltaTime );
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      m_particleLayout->addMidiEvent( midiEvent );
    }

    const ParticleLayoutData_t& getParticleOptions() const
    {
      return m_particleLayout->getParticleOptions();
    }

    std::deque< TimedParticle_t* >& getParticles() const
    {
      return m_particleLayout->getParticles();
    }

    void drawMenu()
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

#ifdef DEBUG
          if ( ImGui::RadioButton( "Test", m_particleLayout->getType() == E_LayoutType::E_TestLayout ) )
            changeLayout< TestParticleLayout >();
#endif
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::Separator();
      m_particleLayout->drawMenu();
    }

  private:

    // save settings between changes to make editing less frustrating
    template < typename T >
    void changeLayout()
    {
      m_tempSettings[ SerialHelper::serializeEnum( m_particleLayout->getType() ) ] = m_particleLayout->serialize();
      m_particleLayout.reset( new T( m_globalInfo ) );

      const auto newLayoutName = SerialHelper::serializeEnum( m_particleLayout->getType() );
      if ( m_tempSettings.contains( newLayoutName ) )
        m_particleLayout->deserialize( m_tempSettings[ newLayoutName ] );
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    std::unique_ptr< IParticleLayout > m_particleLayout;

    nlohmann::json m_tempSettings;
  };

}