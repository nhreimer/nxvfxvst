#pragma once

#include "helpers/MenuHelper.hpp"
#include "models/data/ParticleLineData_t.hpp"
#include "shapes/GradientLine.hpp"
#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class ParticleFullMeshLineModifier final : public IParticleModifier
  {
  public:

    explicit ParticleFullMeshLineModifier( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    nlohmann::json serialize() const override
    {
      return
      {
      { "type", getType() },

         { "isActive", m_data.isActive },
        { "lineThickness", m_data.lineThickness }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      m_data.isActive = j.value( "isActive", false );
      m_data.lineThickness = j.value( "lineThickness", 1.0f );
    }

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    E_ModifierType getType() const override { return E_FullMeshModifier; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Full Mesh Lines" ) )
      {
        ImGui::Checkbox( "Connect##1", &m_data.isActive );
        ImGui::SliderFloat( "Thickness##1", &m_data.lineThickness, 1.f, 100.f, "Thickness %0.2f" );

        MenuHelper::drawBlendOptions( m_data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override
    {}

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override
    {
      for ( int i = 0; i < particles.size(); ++i )
      {
        for ( int y = i + 1; y < particles.size(); ++y )
        {
          auto * line = dynamic_cast< GradientLine* >( outArtifacts.emplace_back( new GradientLine() ) );
          line->setStart( particles[ i ]->shape.getPosition() );
          line->setEnd( particles[ y ]->shape.getPosition() );
          line->setWidth( m_data.lineThickness );

          if ( particles[ y ]->timeLeft > particles[ i ]->timeLeft )
          {
            line->setGradient( particles[ y ]->shape.getFillColor(),
                             particles[ i ]->shape.getFillColor() );
          }
          else
          {
            line->setGradient( particles[ i ]->shape.getFillColor(),
                              particles[ y ]->shape.getFillColor() );
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