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
      EXPAND_SHADER_IMGUI(PARTICLE_LINE_MODIFIER_PARAMS, m_data)

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  nlohmann::json ParticleSequentialLineModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(PARTICLE_LINE_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ParticleSequentialLineModifier::deserialize( const nlohmann::json& j )
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
            m_data.curvature.first,
            m_data.lineSegments.first ) ) );

        line->setWidth( m_data.lineThickness.first );

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