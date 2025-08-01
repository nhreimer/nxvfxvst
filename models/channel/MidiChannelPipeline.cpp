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

#include "models/channel/MidiChannelPipeline.hpp"

namespace nx
{

  MidiChannelPipeline::MidiChannelPipeline( PipelineContext& context, const int32_t channelId )
    : ChannelPipeline( context, channelId )
  {}

  void MidiChannelPipeline::processMidiEvent( const Midi_t& midiEvent ) const
  {
    m_particleLayout.processMidiEvent( midiEvent );

    m_modifierPipeline.processMidiEvent( midiEvent );

    // notify all shaders of an incoming event
    // which can be used for synchronizing effects on midi hits
    m_shaderPipeline.processMidiEvent( midiEvent );
  }

  void MidiChannelPipeline::update( const sf::Time& deltaTime ) const
  {
    m_particleLayout.update( deltaTime );
    m_modifierPipeline.update( deltaTime );
    m_shaderPipeline.update( deltaTime );
  }

  void MidiChannelPipeline::drawMenu()
  {
    ImGui::Text( "Particle count: %ld", m_particleLayout.getParticles().size() );
    ImGui::Separator();
    m_particleLayout.drawMidiMenu();

    ImGui::Separator();
    m_modifierPipeline.drawMenu();

    ImGui::Separator();
    m_shaderPipeline.drawMenu();

    ImGui::Separator();
    drawChannelPipelineMenu();
    ImGui::Separator();
  }
}