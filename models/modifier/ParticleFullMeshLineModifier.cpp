#pragma once

#include "models/modifier/ParticleFullMeshLineModifier.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json ParticleFullMeshLineModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(PARTICLE_LINE_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleFullMeshLineModifier::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(PARTICLE_LINE_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleFullMeshLineModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Full Mesh Lines" ) )
    {
      EXPAND_SHADER_IMGUI(PARTICLE_LINE_MODIFIER_PARAMS, m_data)

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleFullMeshLineModifier::modify(
     const ParticleLayoutData_t& particleLayoutData,
     std::deque< TimedParticle_t* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
  {
    for ( int i = 0; i < particles.size(); ++i )
    {
      for ( int y = i + 1; y < particles.size(); ++y )
      {
        auto * line = new CurvedLine(
          particles[ i ]->shape.getPosition(),
          particles[ y ]->shape.getPosition(),
          m_data.curvature.first,
          m_data.lineSegments.first );

        line->setWidth( m_data.lineThickness.first );

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

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////

  void ParticleFullMeshLineModifier::setLineColors( CurvedLine * line,
                      const TimedParticle_t * pointA,
                      const TimedParticle_t * pointB ) const
  {
    if ( m_data.useParticleColors.first )
    {
      line->setGradient( pointA->shape.getFillColor(), pointB->shape.getFillColor() );
    }
    else
    {
      line->setGradient( m_data.lineColor.first, m_data.otherLineColor.first );
    }
  }

}