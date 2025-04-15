#pragma once

#include "shapes/GradientLine.hpp"
#include "models/data/ParticleLineData_t.hpp"

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class ParticleSequentialLineModifier final : public IParticleModifier
  {
  public:
    explicit ParticleSequentialLineModifier( const GlobalInfo_t& globablInfo )
      : m_globalInfo( globablInfo )
    {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Sequential Lines" ) )
      {
        ImGui::Checkbox( "Connect##1", &m_data.isActive );
        ImGui::SliderFloat( "Thickness##1", &m_data.lineThickness, 1.f, 100.f, "Thickness %0.2f" );

        MenuHelper::drawBlendOptions( m_data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    bool isActive() const override { return m_isActive; }

    void processMidiEvent(const Midi_t &midiEvent) override
    {}

    nlohmann::json serialize() const override
    {
      return
      {
        { "type", getType() },

          { "isActive", m_data.isActive },
          { "lineThickness", m_data.lineThickness },
        { "blendMode", SerialHelper::convertBlendModeToString( m_data.blendMode ) }

      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      m_data.isActive = j.value( "isActive", false );
      m_data.lineThickness = j.value( "lineThickness", 1.0f );
      m_data.blendMode = SerialHelper::convertBlendModeFromString( j.value( "blendMode", "BlendAdd" ) );
    }

    E_ModifierType getType() const override { return E_ModifierType::E_SequentialModifier; }

    void update( const sf::Time &deltaTime ) override
    {}

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override
    {
      for ( int i = 0; i < particles.size(); ++i )
      {
        if ( m_data.isActive && i > 0 )
        {
          auto * line = dynamic_cast< GradientLine* >( outArtifacts.emplace_back( new GradientLine() ) );
          line->setStart( particles[ i - 1 ]->shape.getPosition() );
          line->setEnd( particles[ i ]->shape.getPosition() );
          line->setWidth( m_data.lineThickness );

          if ( particles[ i ]->timeLeft > particles[ i - 1 ]->timeLeft )
          {
            line->setGradient( particles[ i ]->shape.getFillColor(),
                             particles[ i - 1 ]->shape.getFillColor() );
          }
          else
          {
            line->setGradient( particles[ i - 1 ]->shape.getFillColor(),
                              particles[ i ]->shape.getFillColor() );
          }
        }
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    bool m_isActive { true };

    ParticleLineData_t m_data;

  };

}