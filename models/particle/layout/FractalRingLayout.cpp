#include "models/particle/layout/FractalRingLayout.hpp"

namespace nx
{


  [[nodiscard]]
  nlohmann::json FractalRingLayout::serialize() const
  {
    nlohmann::json j =
    {
     { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "depthLimit" ] = m_data.depthLimit;
    j[ "radialSpread" ] = m_data.radialSpread;
    j[ "baseRingCount" ] = m_data.baseRingCount;
    j[ "radiusAdjustment" ] = m_data.radiusAdjustment;
    j[ "delayFractalFadesMultiplier" ] = m_data.delayFractalFadesMultiplier;
    j[ "enableFractalFades" ] = m_data.enableFractalFades;
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void FractalRingLayout::deserialize(const nlohmann::json &j)
  {
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
    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );
    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void FractalRingLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    const sf::Vector2f pos =
    {
      m_ctx.globalInfo.windowHalfSize.x,
      m_ctx.globalInfo.windowHalfSize.y
    };

    // auto * p = createParticle( midiEvent, pos, m_data.radius );
    // p->shape.setPosition( pos );
    // spawnFractalRing( midiEvent, m_data.depthLimit, m_data.radius, pos );

    // Only spawn one level of fractal on each MIDI note
    spawnFractalRing(midiEvent, m_currentDepth, m_particleGeneratorManager.getParticleGenerator()->getData().radius, pos);

    // Advance or reset depth
    switch (m_data.fractalDepthTraversalMode)
    {
      case E_FractalDepthTraversalMode::E_Forward:
        ++m_currentDepth;
        if (m_currentDepth > m_data.depthLimit)
          m_currentDepth = 1;
        break;

      case E_FractalDepthTraversalMode::E_Reverse:
        m_currentDepth--;
        if (m_currentDepth < 1)
          m_currentDepth = m_data.depthLimit;
        break;

      case E_FractalDepthTraversalMode::E_PingPong:
        m_currentDepth += m_data.depthDirection;
        if (m_currentDepth >= m_data.depthLimit)
        {
          m_currentDepth = m_data.depthLimit;
          m_data.depthDirection = -1;
        }
        else if (m_currentDepth <= 1)
        {
          m_currentDepth = 1;
          m_data.depthDirection = 1;
        }
        break;
      default:
        LOG_ERROR( "Unknown Fractal Traversal Direction!" );
        break;
    }
  }

  void FractalRingLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Fractal Ring Layout" ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::Separator();
      ImGui::SliderInt("Spawn Depth", &m_data.depthLimit, 0, 4);
      ImGui::SliderInt("Base Ring Count", &m_data.baseRingCount, 0, 8);
      ImGui::SliderFloat( "Radius Adjustment", &m_data.radiusAdjustment, 0.f, 1.f );
      ImGui::SliderFloat( "Radial Spread", &m_data.radialSpread, 0.f, 5.f );
      ImGui::Checkbox( "Enable Fractal Depth Fade", &m_data.enableFractalFades );
      ImGui::SliderFloat( "Fractal Depth Fade Offset", &m_data.delayFractalFadesMultiplier, 0.f, 5.f );

      ImGui::SeparatorText( "Fractal Depth Traversal" );

      if ( ImGui::RadioButton( "Forward##1", m_data.fractalDepthTraversalMode == E_FractalDepthTraversalMode::E_Forward ) )
      {
        m_data.fractalDepthTraversalMode = E_FractalDepthTraversalMode::E_Forward;
      }
      else if ( ImGui::RadioButton( "Reverse##1", m_data.fractalDepthTraversalMode == E_FractalDepthTraversalMode::E_Reverse ) )
      {
        m_data.fractalDepthTraversalMode = E_FractalDepthTraversalMode::E_Reverse;
      }
      else if ( ImGui::RadioButton( "Ping-Pong##1", m_data.fractalDepthTraversalMode == E_FractalDepthTraversalMode::E_PingPong ) )
      {
        m_data.fractalDepthTraversalMode = E_FractalDepthTraversalMode::E_PingPong;
      }

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
    if (depth <= 0)
      return;

    const float lastRadius = adjustedRadius / m_data.radiusAdjustment;

    const float angleStep = NX_TAU / static_cast< float >(m_data.baseRingCount);

    auto& particleData = m_particleGeneratorManager.getParticleGenerator()->getData();

    for (int i = 0; i < m_data.baseRingCount; ++i)
    {
      const float angle = static_cast< float >(i) * angleStep;

      sf::Vector2f pos =
      {
        lastPosition.x + std::cos(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread ),
        lastPosition.y + std::sin(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread )
      };

      auto* p = createParticle(midiEvent, adjustedRadius);
      p->setPosition(pos);

      if ( m_data.enableFractalFades )
      {
        p->setExpirationTimeInSeconds(
          p->getExpirationTimeInSeconds() - static_cast< int32_t >(
          m_data.delayFractalFadesMultiplier *
          static_cast< float >(depth) *
          static_cast< float >(particleData.timeoutInSeconds)) );
      }

      spawnFractalRing(
        midiEvent,
        depth - 1,
        adjustedRadius * m_data.radiusAdjustment,
        pos
      );
    }

  }

  IParticle * FractalRingLayout::createParticle( const Midi_t& midiEvent,
                                                 const float adjustedRadius )
  {
    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent,
        m_ctx.globalInfo.elapsedTimeSeconds,
        adjustedRadius ) );

    notifyBehaviorOnSpawn( p );

    return p;
  }

}