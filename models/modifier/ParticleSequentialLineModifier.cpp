#include "models/modifier/ParticleSequentialLineModifier.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  void ParticleSequentialLineModifier::drawMenu()
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

  /////////////////////////////////////////////////////////
  /// PUBLIC
  nlohmann::json ParticleSequentialLineModifier::serialize() const
  {
    return
    {
      { "type", getType() },

        { "isActive", m_data.isActive },
        { "lineThickness", m_data.lineThickness },
      { "blendMode", SerialHelper::convertBlendModeToString( m_data.blendMode ) }

    };
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleSequentialLineModifier::deserialize( const nlohmann::json& j )     {
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

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleSequentialLineModifier::modify(
     const ParticleLayoutData_t& particleLayoutData,
     std::deque< TimedParticle_t* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
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

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  void ParticleSequentialLineModifier::setLineColors( CurvedLine * line,
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

}