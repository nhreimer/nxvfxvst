#include "models/particle/FractalRingLayout.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{


  [[nodiscard]]
  nlohmann::json FractalRingLayout::serialize() const
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "depthLimit" ] = m_data.depthLimit;
    j[ "radialSpread" ] = m_data.radialSpread;
    j[ "baseRingCount" ] = m_data.baseRingCount;
    j[ "radiusAdjustment" ] = m_data.radiusAdjustment;
    j[ "delayFractalFadesMultiplier" ] = m_data.delayFractalFadesMultiplier;
    j[ "enableFractalFades" ] = m_data.enableFractalFades;
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    return j;
  }

  void FractalRingLayout::deserialize(const nlohmann::json &j)
  {
    ParticleHelper::deserialize( m_data, j );
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.depthLimit = j["depthLimit"];
      m_data.radialSpread = j["radialSpread"];
      m_data.baseRingCount = j["baseRingCount"];
      m_data.radiusAdjustment = j["radiusAdjustment"];
      m_data.delayFractalFadesMultiplier = j["delayFractalFadesMultiplier"];
      m_data.enableFractalFades = j["enableFractalFades"];
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j["behaviors"] );
  }

  void FractalRingLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    // const float angle = ( ( midiEvent.pitch / 127.f ) * NX_TAU );

    const sf::Vector2f pos =
    {
      // m_globalInfo.windowHalfSize.x + std::cos( angle ) * m_data.radius,
      // m_globalInfo.windowHalfSize.y + std::sin( angle ) * m_data.radius
      m_ctx.globalInfo.windowHalfSize.x,
      m_ctx.globalInfo.windowHalfSize.y
    };

    auto * p = createParticle( midiEvent, pos, m_data.radius );
    p->shape.setPosition( pos );
    spawnFractalRing( midiEvent, m_data.depthLimit, m_data.radius, pos );
  }

  void FractalRingLayout::update( const sf::Time &deltaTime )
  {
    for ( auto i = 0; i < m_particles.size(); ++i )
    {
      const auto& timeParticle = m_particles[ i ];
      timeParticle->timeLeft += deltaTime.asMilliseconds();
      const auto percentage = static_cast< float >( timeParticle->timeLeft ) /
                         static_cast< float >( m_data.timeoutInMS );

      if ( percentage < 1.f )
      {
        const auto nextColor =
          ColorHelper::getNextColor(
            timeParticle->initialColor,
            m_data.endColor,
            percentage );

        timeParticle->shape.setFillColor( nextColor );
      }
      else
      {
        delete m_particles[ i ];
        m_particles.erase( m_particles.begin() + i );
      }
    }
  }

  void FractalRingLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Fractal Ring Layout" ) )
    {
      ParticleHelper::drawMenu( m_data );
      ImGui::Separator();
      ImGui::SliderInt("Spawn Depth", &m_data.depthLimit, 0, 4);
      ImGui::SliderInt("Base Ring Count", &m_data.baseRingCount, 0, 8);
      ImGui::SliderFloat( "Radius Adjustment", &m_data.radiusAdjustment, 0.f, 1.f );
      ImGui::SliderFloat( "Radial Spread", &m_data.radialSpread, 0.f, 5.f );
      ImGui::Checkbox( "Enable Fractal Depth Fade", &m_data.enableFractalFades );
      ImGui::SliderFloat( "Fractal Depth Fade Offset", &m_data.delayFractalFadesMultiplier, 0.f, 5.f );

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }


  void FractalRingLayout::spawnFractalRing( const Midi_t& midiEvent,
                         const int depth,
                         const float adjustedRadius,
                         const sf::Vector2f& lastPosition )
  {
    // base case
    if ( depth <= 0 ) return;

    const float lastRadius = adjustedRadius / m_data.radiusAdjustment;

    const float angleStep = NX_TAU / m_data.baseRingCount;

    for (int i = 0; i < m_data.baseRingCount; ++i)
    {
      const float angle = i * angleStep;

      sf::Vector2f pos =
      {
        lastPosition.x + std::cos(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread ),
        lastPosition.y + std::sin(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread )
      };

      auto* p = createParticle(midiEvent, pos, adjustedRadius);
      p->shape.setPosition(pos);

      if ( m_data.enableFractalFades )
        p->timeLeft -= ( m_data.delayFractalFadesMultiplier * depth * m_data.timeoutInMS );

      spawnFractalRing(
        midiEvent,
        depth - 1,
        adjustedRadius * m_data.radiusAdjustment,
        pos
      );
    }

  }

  TimedParticle_t* FractalRingLayout::createParticle( const Midi_t& midiEvent,
                                   const sf::Vector2f& position,
                                   const float adjustedRadius )
  {
    auto * p = m_particles.emplace_back( new TimedParticle_t() );
    p->spawnTime = m_ctx.globalInfo.elapsedTimeSeconds;

    p->initialColor = ColorHelper::getColorPercentage(
        m_data.startColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

    auto& shape = p->shape;
    shape.setPosition( position );
    shape.setRadius( adjustedRadius );
    shape.setFillColor( p->initialColor );

    shape.setOutlineThickness( m_data.outlineThickness );
    shape.setOutlineColor( m_data.outlineColor );
    shape.setOrigin( shape.getGlobalBounds().size / 2.f );

    return p;
  }

}