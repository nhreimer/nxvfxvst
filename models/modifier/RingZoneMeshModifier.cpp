#include "models/modifier/RingZoneMeshModifier.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json RingZoneMeshModifier::serialize() const
  {
    return
    {
      { "type", SerialHelper::serializeEnum( getType() ) },
      { "isActive", m_data.isActive },
      { "ringSpacing", m_data.ringSpacing },
      { "drawRings", m_data.drawRings },
      { "drawSpokes", m_data.drawSpokes },
      { "lineColor", SerialHelper::convertColorToJson( m_data.lineColor ) },
      { "otherLineColor", SerialHelper::convertColorToJson( m_data.otherLineColor ) },
      { "lineWidth", m_data.lineThickness },
      { "pulseSpeed", m_data.pulseSpeed },
      { "minAlpha", m_data.minAlpha },
       { "maxAlpha", m_data.maxAlpha },
       { "enablePulse", m_data.enablePulse }
    };
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void RingZoneMeshModifier::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.isActive = j[ "isActive" ].get<bool>();
      m_data.ringSpacing = j.at( "ringSpacing" ).get<float>();
      m_data.drawRings = j.at( "drawRings" ).get<bool>();
      m_data.lineThickness = j.at( "lineWidth" ).get<float>();
      m_data.pulseSpeed = j.at( "pulseSpeed" ).get<float>();
      m_data.minAlpha = j.at( "minAlpha" ).get<float>();
      m_data.maxAlpha = j.at( "maxAlpha" ).get<float>();
      m_data.enablePulse = j.at( "enablePulse" ).get<bool>();
      m_data.lineColor = SerialHelper::convertColorFromJson( j.at( "lineColor" ) );
      m_data.otherLineColor = SerialHelper::convertColorFromJson( j.at( "otherLineColor" ) );
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
      ImGui::SliderFloat("Ring Spacing", &m_data.ringSpacing, 20.f, 200.f);
      ImGui::Checkbox("Draw Ring Loops", &m_data.drawRings);
      ImGui::Checkbox("Draw Radials", &m_data.drawSpokes);
      // ImGui::SliderFloat("Line Width", &m_data.lineWidth, 1.f, 50.f);
      //
      // ImVec4 color = m_data.lineColor;
      // if (ImGui::ColorEdit4("Line Color", reinterpret_cast< float * >(&color)))
      //   m_data.lineColor = color;
      //
      // color = m_data.otherLineColor;
      // if (ImGui::ColorEdit4("Other Line Color", reinterpret_cast< float * >(&color)))
      //   m_data.otherLineColor = color;

      ImGui::SeparatorText("Pulse Alpha");
      ImGui::Checkbox("Enable Pulse", &m_data.enablePulse);
      if (m_data.enablePulse)
      {
        ImGui::SliderFloat("Pulse Speed (Hz)", &m_data.pulseSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Min Alpha", &m_data.minAlpha, 0.f, 255.f);
        ImGui::SliderFloat("Max Alpha", &m_data.maxAlpha, 0.f, 255.f);
      }

      ImGui::SeparatorText("Line Options");

      ImGui::SliderFloat( "Thickness##1", &m_data.lineThickness, 1.f, 100.f, "Thickness %0.2f" );
      ImGui::SliderFloat( "Curvature##1", &m_data.curvature, -NX_PI, NX_PI, "Curvature %0.2f" );
      ImGui::SliderInt( "Segments##1", &m_data.lineSegments, 1, 150, "Segments %d" );

      ImGui::Checkbox( "Use Particle Colors", &m_data.useParticleColors );

      if ( !m_data.useParticleColors )
      {
        ColorHelper::drawImGuiColorEdit4( "Line Color", m_data.lineColor );
        ColorHelper::drawImGuiColorEdit4( "Other Line Color", m_data.otherLineColor );
      }

      //ImGui::SeparatorText( "Blend Options" );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void RingZoneMeshModifier::modify(const ParticleLayoutData_t &, std::deque< TimedParticle_t * > &particles,
              std::deque< sf::Drawable * > &outArtifacts)
  {
    float alpha = m_data.lineColor.a; // default static alpha

    if (m_data.enablePulse)
    {
      float time = m_ctx.globalInfo.elapsedTimeSeconds;
      float t = std::sin(time * m_data.pulseSpeed * NX_TAU); // [-1, 1]
      float normalized = 0.5f * (t + 1.f); // [0, 1]
      alpha = m_data.minAlpha + (m_data.maxAlpha - m_data.minAlpha) * normalized;
    }

    auto pulsedColor = m_data.lineColor;
    pulsedColor.a = static_cast< uint8_t >(alpha);

    auto otherPulsedColor = m_data.otherLineColor;
    otherPulsedColor.a = static_cast< uint8_t >( alpha );

    //auto *lines = new sf::VertexArray(sf::PrimitiveType::Lines);
    //auto * lines = static_cast< GradientLine * >(outArtifacts.emplace_back(new GradientLine()));
    const sf::Vector2f &center = m_ctx.globalInfo.windowHalfSize;

    // Step 1: Group particles into rings
    std::map< int, std::vector< TimedParticle_t * > > rings;
    for (auto *p: particles)
    {
      const float dist = length(p->shape.getPosition() - center);
      int ringIdx = static_cast< int >(dist / m_data.ringSpacing);
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

      if (m_data.drawRings)
      {
        for (size_t i = 0; i < ringParticles.size(); ++i)
        {
          auto *p1 = ringParticles[ i ];
          auto *p2 = ringParticles[ (i + 1) % ringParticles.size() ]; // wrap around

          auto * line = new CurvedLine( p1->shape.getPosition(),
                                        p2->shape.getPosition(),
                                        m_data.curvature,
                                        m_data.lineSegments );

          line->setWidth( m_data.lineThickness );

          if ( p1->timeLeft > p2->timeLeft )
            setLineColors( line, p1, p2, pulsedColor );
          else
            setLineColors( line, p2, p1, otherPulsedColor );

          outArtifacts.push_back( line );
        }
      }

      if (m_data.drawSpokes && ringIdx > 0)
      {
        auto &prevRing = rings[ ringIdx - 1 ];
        const size_t minCount = std::min(ringParticles.size(), prevRing.size());

        for (size_t i = 0; i < minCount; ++i)
        {

          auto * line = new CurvedLine( ringParticles[ i ]->shape.getPosition(),
                                        prevRing[ i ]->shape.getPosition(),
                                        m_data.curvature,
                                        m_data.lineSegments );

          line->setWidth( m_data.lineThickness );

          if ( ringParticles[ i ]->timeLeft > prevRing[ i ]->timeLeft )
            setLineColors( line, ringParticles[ i ], prevRing[ i ], pulsedColor );
          else
            setLineColors( line, prevRing[ i ], ringParticles[ i ], otherPulsedColor );

          outArtifacts.push_back( line );

          // outArtifacts.emplace_back(
          //   new GradientLine( ringParticles[ i ]->shape.getPosition(),
          //                      prevRing[ i ]->shape.getPosition(),
          //                      m_data.lineThickness,
          //                      pulsedColor,
          //                      otherPulsedColor ) );
          // lines->append(sf::Vertex(ringParticles[ i ]->shape.getPosition(), pulsedColor));
          // lines->append(sf::Vertex(prevRing[ i ]->shape.getPosition(), pulsedColor));
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
    if ( m_data.useParticleColors )
    {
      line->setGradient( pointA->shape.getFillColor(), pointB->shape.getFillColor() );
    }
    else if ( m_data.enablePulse )
    {
      line->setGradient( m_data.lineColor, pulsedColor );
    }
    else
    {
      line->setGradient( m_data.lineColor, m_data.otherLineColor );
    }
  }

} // namespace nx
