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

#include "models/particle/layout/LissajousCurveLayout.hpp"

#include "helpers/SerialHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json LissajousCurveLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    EXPAND_SHADER_PARAMS_TO_JSON(LISSAJOUS_CURVE_LAYOUT_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void LissajousCurveLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(LISSAJOUS_CURVE_LAYOUT_PARAMS)
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

  void LissajousCurveLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Lissajous Curve Layout" ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      EXPAND_SHADER_IMGUI(LISSAJOUS_CURVE_LAYOUT_PARAMS, m_data)

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Separator();
    }
  }

  void LissajousCurveLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    const float a = m_data.phaseAStep.first + static_cast< float >(midiEvent.pitch % 4); // X frequency
    const float b = m_data.phaseBStep.first + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency

    const float localT = m_ctx.globalInfo.elapsedTimeSeconds;

    const float x = ( static_cast< float >( m_ctx.globalInfo.windowSize.x ) * m_data.phaseSpread.first ) * sin(a * localT + m_data.phaseDelta.first);
    const float y = ( static_cast< float >( m_ctx.globalInfo.windowSize.y ) * m_data.phaseSpread.first ) * sin(b * localT);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( { x, y } );
    ParticleLayoutBase::notifyBehaviorOnSpawn( p );
  }

  // sf::Vector2f LissajousCurveLayout::getNextPosition( const Midi_t & midiEvent ) const
  // {
  //   const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
  //   const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency
  //
  //   // this is also the phase
  //   const float t = m_ctx.globalInfo.elapsedTimeSeconds;
  //
  //   const float x = ( m_ctx.globalInfo.windowSize.x * m_data.phaseSpread ) * sin(a * t + m_data.phaseDelta);
  //   const float y = ( m_ctx.globalInfo.windowSize.y * m_data.phaseSpread ) * sin(b * t);
  //
  //   return { x, y };
  // }


}