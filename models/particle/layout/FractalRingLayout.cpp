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

    EXPAND_SHADER_PARAMS_TO_JSON(FRACTAL_RING_LAYOUT_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void FractalRingLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(FRACTAL_RING_LAYOUT_PARAMS)
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
      case layout::fractalring::E_FractalDepthTraversalMode::E_Forward:
        ++m_currentDepth;
        if (m_currentDepth > m_data.depthLimit.first)
          m_currentDepth = 1;
        break;

      case layout::fractalring::E_FractalDepthTraversalMode::E_Reverse:
        m_currentDepth--;
        if (m_currentDepth < 1)
          m_currentDepth = m_data.depthLimit.first;
        break;

      case layout::fractalring::E_FractalDepthTraversalMode::E_PingPong:
        m_currentDepth += m_data.depthDirection.first;
        if (m_currentDepth >= m_data.depthLimit.first)
        {
          m_currentDepth = m_data.depthLimit.first;
          m_data.depthDirection.first = -1;
        }
        else if (m_currentDepth <= 1)
        {
          m_currentDepth = 1;
          m_data.depthDirection.first = 1;
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
      EXPAND_SHADER_IMGUI(FRACTAL_RING_LAYOUT_PARAMS, m_data)
      ImGui::SeparatorText( "Fractal Depth Traversal" );

      if ( ImGui::RadioButton(
        "Forward##1",
        m_data.fractalDepthTraversalMode == layout::fractalring::E_FractalDepthTraversalMode::E_Forward ) )
      {
        m_data.fractalDepthTraversalMode = layout::fractalring::E_FractalDepthTraversalMode::E_Forward;
      }
      else if ( ImGui::RadioButton(
        "Reverse##1",
        m_data.fractalDepthTraversalMode == layout::fractalring::E_FractalDepthTraversalMode::E_Reverse ) )
      {
        m_data.fractalDepthTraversalMode = layout::fractalring::E_FractalDepthTraversalMode::E_Reverse;
      }
      else if ( ImGui::RadioButton(
        "Ping-Pong##1",
        m_data.fractalDepthTraversalMode == layout::fractalring::E_FractalDepthTraversalMode::E_PingPong ) )
      {
        m_data.fractalDepthTraversalMode = layout::fractalring::E_FractalDepthTraversalMode::E_PingPong;
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

    const float lastRadius = adjustedRadius / m_data.radiusAdjustment.first;

    const float angleStep = NX_TAU / static_cast< float >(m_data.baseRingCount.first);

    auto& particleData = m_particleGeneratorManager.getParticleGenerator()->getData();

    for (int i = 0; i < m_data.baseRingCount.first; ++i)
    {
      const float angle = static_cast< float >(i) * angleStep;

      sf::Vector2f pos =
      {
        lastPosition.x + std::cos(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread.first ),
        lastPosition.y + std::sin(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread.first )
      };

      auto* p = createParticle(midiEvent, adjustedRadius);
      p->setPosition(pos);

      if ( m_data.enableFractalFades.first )
      {
        p->setExpirationTimeInSeconds(
          p->getExpirationTimeInSeconds() - static_cast< int32_t >(
          m_data.delayFractalFadesMultiplier.first *
          static_cast< float >(depth) *
          static_cast< float >(particleData.timeoutInSeconds)) );
      }

      spawnFractalRing(
        midiEvent,
        depth - 1,
        adjustedRadius * m_data.radiusAdjustment.first,
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