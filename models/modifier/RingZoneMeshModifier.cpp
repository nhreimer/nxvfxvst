#include "models/modifier/RingZoneMeshModifier.hpp"

#include "helpers/ColorHelper.hpp"

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
  void RingZoneMeshModifier::modify(const ParticleLayoutData_t &, std::deque< TimedParticle_t * > &particles,
              std::deque< sf::Drawable * > &outArtifacts)
  {
    float alpha = m_data.lineColor.first.a; // default static alpha

    if (m_data.enablePulse.first)
    {
      float time = m_ctx.globalInfo.elapsedTimeSeconds;
      float t = std::sin(time * m_data.pulseSpeed.first * NX_TAU); // [-1, 1]
      float normalized = 0.5f * (t + 1.f); // [0, 1]
      alpha = m_data.minAlpha.first + (m_data.maxAlpha.first - m_data.minAlpha.first) * normalized;
    }

    auto pulsedColor = m_data.lineColor.first;
    pulsedColor.a = static_cast< uint8_t >(alpha);

    auto otherPulsedColor = m_data.otherLineColor.first;
    otherPulsedColor.a = static_cast< uint8_t >( alpha );

    //auto *lines = new sf::VertexArray(sf::PrimitiveType::Lines);
    //auto * lines = static_cast< GradientLine * >(outArtifacts.emplace_back(new GradientLine()));
    const sf::Vector2f &center = m_ctx.globalInfo.windowHalfSize;

    // Step 1: Group particles into rings
    std::map< int, std::vector< TimedParticle_t * > > rings;
    for (auto *p: particles)
    {
      const float dist = length(p->shape.getPosition() - center);
      int ringIdx = static_cast< int >(dist / m_data.ringSpacing.first);
      rings[ ringIdx ].push_back(p);
    }

    // Step 2: Draw rings and spokes
    // TimedParticle_t *prevInRing = nullptr;

    for (auto &[ ringIdx, ringParticles ]: rings)
    {
      if (ringParticles.size() < 2)
        continue;

      // Sort particles clockwise by angle
      std::ranges::sort(
      ringParticles, [ & ](TimedParticle_t *a, TimedParticle_t *b)
      { return angleFromCenter(center, a->shape.getPosition()) < angleFromCenter(center, b->shape.getPosition()); });

      if (m_data.drawRings.first)
      {
        for (size_t i = 0; i < ringParticles.size(); ++i)
        {
          auto *p1 = ringParticles[ i ];
          auto *p2 = ringParticles[ (i + 1) % ringParticles.size() ]; // wrap around

          auto * line = new CurvedLine( p1->shape.getPosition(),
                                        p2->shape.getPosition(),
                                        m_data.curvature.first,
                                        m_data.lineSegments.first );

          line->setWidth( m_data.lineThickness.first );

          if ( p1->timeLeft > p2->timeLeft )
            setLineColors( line, p1, p2, pulsedColor );
          else
            setLineColors( line, p2, p1, otherPulsedColor );

          outArtifacts.push_back( line );
        }
      }

      if (m_data.drawSpokes.first && ringIdx > 0)
      {
        auto &prevRing = rings[ ringIdx - 1 ];
        const size_t minCount = std::min(ringParticles.size(), prevRing.size());

        for (size_t i = 0; i < minCount; ++i)
        {

          auto * line = new CurvedLine( ringParticles[ i ]->shape.getPosition(),
                                        prevRing[ i ]->shape.getPosition(),
                                        m_data.curvature.first,
                                        m_data.lineSegments.first );

          line->setWidth( m_data.lineThickness.first );

          if ( ringParticles[ i ]->timeLeft > prevRing[ i ]->timeLeft )
            setLineColors( line, ringParticles[ i ], prevRing[ i ], pulsedColor );
          else
            setLineColors( line, prevRing[ i ], ringParticles[ i ], otherPulsedColor );

          outArtifacts.push_back( line );
        }
      }
    }
  }


  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  void RingZoneMeshModifier::setLineColors( CurvedLine * line,
                      const TimedParticle_t * pointA,
                      const TimedParticle_t * pointB,
                      const sf::Color pulsedColor ) const
  {
    if ( m_data.useParticleColors.first )
    {
      line->setGradient( pointA->shape.getFillColor(), pointB->shape.getFillColor() );
    }
    else if ( m_data.enablePulse.first )
    {
      line->setGradient( m_data.lineColor.first, pulsedColor );
    }
    else
    {
      line->setGradient( m_data.lineColor.first, m_data.otherLineColor.first );
    }
  }

} // namespace nx
