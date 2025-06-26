#include "models/modifier/ParticleSequentialLineModifier.hpp"

#include "helpers/LineHelper.hpp"

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
     const sf::BlendMode& blendMode,
     std::deque< IParticle* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
  {
    for ( int i = 0; i < particles.size(); ++i )
    {
      if ( m_data.isActive && i > 0 )
      {
        auto * line = dynamic_cast< CurvedLine* >( outArtifacts.emplace_back(
          new CurvedLine( particles[ i - 1 ]->getPosition(),
            particles[ i ]->getPosition(),
            m_data.curvature.first,
            m_data.lineSegments.first ) ) );

        line->setWidth( m_data.lineThickness.first );

        if ( m_data.useParticleColors.first )
        {
          LineHelper::updateLineColors( line,
            particles[ i - 1 ],
            particles[ i ],
            m_data.invertColorTime.first );
        }
        else
        {
          LineHelper::updateCustomLineColors(
            line,
            particles[ i - 1 ],
            particles[ i ],
            m_data.lineColor.first,
            m_data.otherLineColor.first,
            m_data.invertColorTime.first );
        }
      }
    }
  }
} // namespace nx
