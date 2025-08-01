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

#include "models/particle/layout/LSystemCurveLayout.hpp"

#include "helpers/SerialHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json LSystemCurveLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    EXPAND_SHADER_PARAMS_TO_JSON(LSYSTEM_CURVE_LAYOUT_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void LSystemCurveLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(LSYSTEM_CURVE_LAYOUT_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for layout {}", SerialHelper::serializeEnum( getType() ) );
    }

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j["behaviors"] );
    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );
    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void LSystemCurveLayout::drawMenu()
  {
    ImGui::Text("Particles: %ld", m_particles.size());
    ImGui::Separator();
    if (ImGui::TreeNode("L-System Particle Layout"))
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::Separator();

      EXPAND_SHADER_IMGUI(LSYSTEM_CURVE_LAYOUT_PARAMS, m_data)

      ImGui::SeparatorText( "Branch Mode" );
      if ( ImGui::RadioButton( "Both", m_data.branchMode == layout::lsystem::E_LSystemBranchMode::E_Both ) )
        m_data.branchMode = layout::lsystem::E_LSystemBranchMode::E_Both;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Left", m_data.branchMode == layout::lsystem::E_LSystemBranchMode::E_LeftOnly ) )
        m_data.branchMode = layout::lsystem::E_LSystemBranchMode::E_LeftOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Right", m_data.branchMode == layout::lsystem::E_LSystemBranchMode::E_RightOnly ) )
        m_data.branchMode = layout::lsystem::E_LSystemBranchMode::E_RightOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Midi Pitch", m_data.branchMode == layout::lsystem::E_LSystemBranchMode::E_MidiPitch ) )
        m_data.branchMode = layout::lsystem::E_LSystemBranchMode::E_MidiPitch;

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void LSystemCurveLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    // 127 - 21 = full pitch range
    if (m_lsystemStack.empty())
    {
      // Start a new tree!
      const auto offsetX = static_cast< float >(midiEvent.pitch) / 106.f;
      const sf::Vector2f position =
      {
        static_cast< float >(m_ctx.globalInfo.windowSize.x) * offsetX,
        static_cast< float >(m_ctx.globalInfo.windowSize.y) * offsetX
      };

      m_currentDepth = 1;
      m_lsystemStack.clear();

      m_lsystemStack.push_back({
        position,
        m_data.initialAngleDeg.first,
        m_data.depth.first,
        midiEvent
      });
    }

    for (int i = 0; i < m_data.stepsPerNote.first && !m_lsystemStack.empty(); ++i)
    {
      auto state = m_lsystemStack.back();
      m_lsystemStack.pop_back();

      expandLSystemStep( midiEvent, state);
    }
  }

  void LSystemCurveLayout::expandLSystemStep(const Midi_t &midiEvent,
                                             const LSystemState_t& state )
  {
    if ( state.depth <= 0 )
    {
      // Final particle placement
      auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent,
        m_ctx.globalInfo.elapsedTimeSeconds ) );

      notifyBehaviorOnSpawn( p );
      return;
    }

    // Extend to the next segment
    const float tightness = 1.f + static_cast< float >(m_data.depth.first - m_currentDepth) * m_data.depthFactor.first;
    const sf::Vector2f dir = MathHelper::polarToCartesian(state.angleDeg,
                              m_data.segmentLength.first * m_data.spreadFactor.first * tightness);
    const sf::Vector2f newPos = ( state.position + dir );

    switch (m_data.branchMode)
    {
      case layout::lsystem::E_LSystemBranchMode::E_Both:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle.first, state.depth - 1, state.midiNote });
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle.first, state.depth - 1, state.midiNote });
        break;

      case layout::lsystem::E_LSystemBranchMode::E_LeftOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle.first, state.depth - 1, state.midiNote });
        break;

      case layout::lsystem::E_LSystemBranchMode::E_RightOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle.first, state.depth - 1, state.midiNote });
        break;

      case layout::lsystem::E_LSystemBranchMode::E_MidiPitch:
        switch (state.midiNote.pitch % 3)
        {
          case 0:
            m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle.first, state.depth - 1, state.midiNote });
            m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle.first, state.depth - 1, state.midiNote });
            break;
          case 1:
            m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle.first, state.depth - 1, state.midiNote });
            break;
          case 2:
            m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle.first, state.depth - 1, state.midiNote });
            break;

          default: break;
        }
        break;

      default: break;
    }
  }

} // namespace nx
