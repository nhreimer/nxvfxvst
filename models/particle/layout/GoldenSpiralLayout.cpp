/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */


#include "models/particle/layout/GoldenSpiralLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json GoldenSpiralLayout::serialize() const
  {
    nlohmann::json j =
    {
          { "type", SerialHelper::serializeEnum( getType() ) }
    };

    EXPAND_SHADER_PARAMS_TO_JSON(GOLDEN_SPIRAL_LAYOUT_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void GoldenSpiralLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(GOLDEN_SPIRAL_LAYOUT_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }

    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void GoldenSpiralLayout::drawMenu()
  {
    ImGui::Text("Particles: %zu", m_particles.size());
    ImGui::Separator();
    if (ImGui::TreeNode("Golden Spiral Layout"))
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::SeparatorText( "Spiral Options" );

      const auto currentRadius = m_data.baseRadius.first;

      EXPAND_SHADER_IMGUI(GOLDEN_SPIRAL_LAYOUT_PARAMS, m_data)

      if ( currentRadius != m_data.baseRadius.first )
      {
        m_timedCursorPosition.setPosition( m_ctx.globalInfo.windowHalfSize,
                                           sf::Vector2f { currentRadius, currentRadius } );
      }

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursorPosition.hasExpired() )
      m_timedCursorPosition.drawPosition();
  }

  void GoldenSpiralLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    const auto pitchSlices =
      static_cast< int32_t >( midiEvent.pitch / static_cast< float >( m_data.depth.first ) );

    for ( int32_t i = 1; i <= m_data.depth.first; ++i )
    {
      auto * p = m_particles.emplace_back(
        m_particleGeneratorManager.getParticleGenerator()->createParticle(
          midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

      const auto pos = getSpiralPosition( i * pitchSlices, m_data.depth.first );
      p->setPosition( pos );
      ParticleLayoutBase::notifyBehaviorOnSpawn( p );
    }
  }

  [[nodiscard]]
  sf::Vector2f GoldenSpiralLayout::getSpiralPosition( const int index,
                                  const int total ) const
  {
    const int i = m_data.spiralInward.first ? (total - 1) - index : index;

    const float angleDeg =
      static_cast< float >( i ) * GOLDEN_ANGLE_DEG * m_data.spiralTightness.first + m_data.angleOffset.first;

    const float angleRad = angleDeg * NX_D2R; //(3.14159265f / 180.f);

    float radius = 0.f;
    if ( m_data.useClamp.first )
    {
      radius = m_data.baseRadius.first * std::pow(m_data.scaleFactor.first, index);
      const float maxR = m_ctx.globalInfo.windowSize.x * 0.45f;

      if ( radius > maxR )
      {
        const float t = (radius - maxR) / maxR;
        radius = maxR + std::sin( t * NX_PI ) * 20.f; // optional ripple-style squish
      }
    }
    else
      radius = m_data.baseRadius.first * std::pow(m_data.scaleFactor.first, i);

    return
    {
      m_ctx.globalInfo.windowHalfSize.x + std::cos( angleRad ) * radius,
      m_ctx.globalInfo.windowHalfSize.y + std::sin( angleRad ) * radius
    };
  }

} // namespace nx
