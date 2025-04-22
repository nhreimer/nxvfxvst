#pragma once

#include "shapes/GradientLine.hpp"
#include "shapes/CurvedLine.hpp"

#include "models/data/ParticleLineData_t.hpp"

#include "helpers/CommonHeaders.hpp"
#include "helpers/ColorHelper.hpp"

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
        ImGui::SliderFloat( "Curvature##1", &m_data.curvature, -NX_PI, NX_PI, "Curvature %0.2f" );
        ImGui::SliderInt( "Segments##1", &m_data.lineSegments, 1, 150, "Segments %d" );

        ImGui::Checkbox( "Use Particle Colors", &m_data.useParticleColors );

        if ( !m_data.useParticleColors )
        {
          ColorHelper::drawImGuiColorEdit4( "Line Color", m_data.lineColor );
          ColorHelper::drawImGuiColorEdit4( "Other Line Color", m_data.otherLineColor );
        }
        //MenuHelper::drawBlendOptions( m_data.blendMode );

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
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value( "isActive", false );
        m_data.lineThickness = j.value( "lineThickness", 1.0f );
        m_data.blendMode = SerialHelper::convertBlendModeFromString( j.value( "blendMode", "BlendAdd" ) );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
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
          auto * line = dynamic_cast< CurvedLine* >( outArtifacts.emplace_back(
            new CurvedLine( particles[ i - 1 ]->shape.getPosition(),
              particles[ i ]->shape.getPosition(),
              m_data.curvature,
              m_data.lineSegments ) ) );

          line->setWidth( m_data.lineThickness );
          line->setColor( sf::Color::White );
          // line->setStart( particles[ i - 1 ]->shape.getPosition() );
          // line->setEnd( particles[ i ]->shape.getPosition() );
          // line->setWidth( m_data.lineThickness );

          if ( particles[ i ]->timeLeft > particles[ i - 1 ]->timeLeft )
          {
            setLineColors( line, particles[ i ], particles[ i - 1 ] );
          }
          else
          {
            setLineColors( line, particles[ i - 1 ], particles[ i ] );
          }
        }
      }
    }

  private:

    void setLineColors( CurvedLine * line,
                        const TimedParticle_t * pointA,
                        const TimedParticle_t * pointB ) const
    {
      if ( m_data.useParticleColors )
      {
        line->setGradient( pointA->shape.getFillColor(), pointB->shape.getFillColor() );
      }
      else
      {
        line->setGradient( m_data.lineColor, m_data.otherLineColor );
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    bool m_isActive { true };

    ParticleLineData_t m_data;

  };

}