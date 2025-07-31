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

#include "models/particle/layout/EllipticalLayout.hpp"

#include "helpers/SerialHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json EllipticalLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    EXPAND_SHADER_PARAMS_TO_JSON(ELLIPTICAL_LAYOUT_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void EllipticalLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(ELLIPTICAL_LAYOUT_PARAMS)
    }

    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void EllipticalLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    if ( m_data.sequential.first ) addSequentialParticle( midiEvent );
    else addParticle( midiEvent );
  }

  void EllipticalLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Elliptical Layout" ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::SeparatorText( "Elliptical Options" );

      const float oldCenterX = m_data.centerOffsetX.first;
      const float oldCenterY = m_data.centerOffsetY.first;

      const float currentRadiusX = m_data.radiusX.first;
      const float currentRadiusY = m_data.radiusY.first;

      EXPAND_SHADER_IMGUI(ELLIPTICAL_LAYOUT_PARAMS, m_data)

      if ( oldCenterX != m_data.centerOffsetX.first || oldCenterY != m_data.centerOffsetY.first ||
           currentRadiusX != m_data.radiusX.first || currentRadiusY != m_data.radiusY.first )
      {
        const sf::Vector2f calibrated { m_data.centerOffsetX.first * static_cast< float >(m_ctx.globalInfo.windowSize.x),
                                        m_data.centerOffsetY.first * static_cast< float >(m_ctx.globalInfo.windowSize.y) };
        m_timedCursor.setPosition( calibrated, sf::Vector2f { m_data.radiusX.first, m_data.radiusY.first }, 100000 );
      }

      ImGui::SeparatorText( "Behaviors" );
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursor.hasExpired() )
      m_timedCursor.drawPosition();
  }

  void EllipticalLayout::addSequentialParticle( const Midi_t& midiEvent )
  {
    const float arcRad = sf::degrees(m_data.arcSpreadDegrees.first).asRadians();
    const float baseAngle = arcRad * m_angleCursor;

    const float rotRad = sf::degrees(m_data.rotationDegrees.first).asRadians();
    const float angle = baseAngle + rotRad;

    const float x = std::cos(angle) * m_data.radiusX.first;
    const float y = std::sin(angle) * m_data.radiusY.first;

    const sf::Vector2f pos =
      m_ctx.globalInfo.windowHalfSize +
        sf::Vector2f { m_data.centerOffsetX.first * m_data.centerOffsetX.first,
                         m_data.centerOffsetY.first * m_data.centerOffsetY.first } +
        sf::Vector2f(x, y);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( pos );

    ParticleLayoutBase::notifyBehaviorOnSpawn( p );

    // advance cursor
    m_angleCursor += 1.f / m_data.slices.first; // 12 evenly spaced notes per full ring
    if (m_angleCursor > 1.f) m_angleCursor -= 1.f;
  }

  void EllipticalLayout::addParticle( const Midi_t& midiEvent )
  {
    const float angleSlice = static_cast< float >( midiEvent.pitch ) * ( 1.f / m_data.slices.first );
    const float arcRad = sf::degrees(m_data.arcSpreadDegrees.first).asRadians();
    const float baseAngle = arcRad * angleSlice;

    const float rotRad = sf::degrees(m_data.rotationDegrees.first).asRadians();
    const float angle = baseAngle + rotRad;

    const float x = std::cos(angle) * m_data.radiusX.first;
    const float y = std::sin(angle) * m_data.radiusY.first;

    const sf::Vector2f calibrated { m_data.centerOffsetX.first * static_cast< float >(m_ctx.globalInfo.windowSize.x),
                                    m_data.centerOffsetY.first * static_cast< float >(m_ctx.globalInfo.windowSize.y) };

    const sf::Vector2f pos =
        calibrated +
        sf::Vector2f(x, y);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( pos );

    ParticleLayoutBase::notifyBehaviorOnSpawn( p );
  }

}