#include "models/modifier/RingZoneMeshModifier.hpp"

#include "helpers/LineHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json RingZoneMeshModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(RING_ZONE_MESH_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void RingZoneMeshModifier::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(RING_ZONE_MESH_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void RingZoneMeshModifier::drawMenu()
  {
    if (ImGui::TreeNode("Ring Zone Mesh Modifier"))
    {
      EXPAND_SHADER_IMGUI(RING_ZONE_MESH_MODIFIER_PARAMS, m_data)
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void RingZoneMeshModifier::modify(const ParticleLayoutData_t & data,
                                    std::deque< IParticle * > &particles,
                                    std::deque< sf::Drawable * > &outArtifacts)
  {
    const sf::Vector2f &center = m_ctx.globalInfo.windowHalfSize;

    // Step 1: Group particles into rings
    std::map< int, std::vector< IParticle * > > rings;
    for (auto *p: particles)
    {
      const float dist = length(p->getPosition() - center);
      int ringIdx = static_cast< int >(dist / m_data.ringSpacing.first);
      rings[ ringIdx ].push_back(p);
    }

    // Step 2: Draw rings and spokes
    for (auto &[ ringIdx, ringParticles ]: rings)
    {
      if (ringParticles.size() < 2)
        continue;

      // Sort particles clockwise by angle
      std::ranges::sort(
      ringParticles, [ & ](IParticle *a, IParticle *b)
      {
        return angleFromCenter(center, a->getPosition()) < angleFromCenter(center, b->getPosition());
      });

      if (m_data.drawRings.first)
      {
        for (size_t i = 0; i < ringParticles.size(); ++i)
        {
          auto *p1 = ringParticles[ i ];
          auto *p2 = ringParticles[ (i + 1) % ringParticles.size() ]; // wrap around

          auto * line = new CurvedLine( p1->getPosition(),
                                        p2->getPosition(),
                                        m_data.curvature.first,
                                        m_data.lineSegments.first );

          line->setWidth( m_data.lineThickness.first );

          if ( p1->getExpirationTimeInSeconds() > p2->getExpirationTimeInSeconds() )
            setLineColors( line, p1, p2 );
          else
            setLineColors( line, p2, p1 );

          outArtifacts.push_back( line );
        }
      }

      if (m_data.drawSpokes.first && ringIdx > 0)
      {
        auto &prevRing = rings[ ringIdx - 1 ];
        const size_t minCount = std::min(ringParticles.size(), prevRing.size());

        for (size_t i = 0; i < minCount; ++i)
        {

          auto * line = new CurvedLine( ringParticles[ i ]->getPosition(),
                                        prevRing[ i ]->getPosition(),
                                        m_data.curvature.first,
                                        m_data.lineSegments.first );

          line->setWidth( m_data.lineThickness.first );

          if ( ringParticles[ i ]->getExpirationTimeInSeconds() > prevRing[ i ]->getExpirationTimeInSeconds() )
            setLineColors( line, ringParticles[ i ], prevRing[ i ] );
          else
            setLineColors( line, prevRing[ i ], ringParticles[ i ] );

          outArtifacts.push_back( line );
        }
      }
    }
  }


  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  void RingZoneMeshModifier::setLineColors( CurvedLine * line,
                      const IParticle * pointA,
                      const IParticle * pointB ) const
  {

    if ( m_data.useParticleColors.first )
    {
      LineHelper::updateLineColors( line,
        pointA,
        pointB,
        m_data.invertColorTime.first );
    }
    else
    {
      LineHelper::updateCustomLineColors(
        line,
        pointA,
        pointB,
        m_data.lineColor.first,
        m_data.otherLineColor.first,
        m_data.invertColorTime.first );
    }
  }
} // namespace nx
