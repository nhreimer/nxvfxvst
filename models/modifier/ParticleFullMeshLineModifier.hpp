#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/data/ParticleLineData_t.hpp"
#include "shapes/CurvedLine.hpp"
#include "helpers/ColorHelper.hpp"

namespace nx
{

  class ParticleFullMeshLineModifier final : public IParticleModifier
  {
  public:

    explicit ParticleFullMeshLineModifier( PipelineContext& context )
      : m_ctx( context )
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
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value( "isActive", false );
        m_data.lineThickness = j.value( "lineThickness", 1.0f );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    E_ModifierType getType() const override { return E_ModifierType::E_FullMeshModifier; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Full Mesh Lines" ) )
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
          auto * line = new CurvedLine(
            particles[ i ]->shape.getPosition(),
            particles[ y ]->shape.getPosition(),
            m_data.curvature,
            m_data.lineSegments );

          line->setWidth( m_data.lineThickness );

          if ( particles[ y ]->timeLeft > particles[ i ]->timeLeft )
          {
            setLineColors( line, particles[ y ], particles[ i ] );
          }
          else
          {
            setLineColors( line, particles[ i ], particles[ y ] );
          }

          outArtifacts.push_back( line );
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

    PipelineContext& m_ctx;

    bool m_isActive { true };
    ParticleLineData_t m_data;

  };

}