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

#pragma once

#include <imgui.h>

#include "vst/analysis/FFTBuffer.hpp"

#include "models/channel/ChannelPipeline.hpp"
#include "models/data/PipelineContext.hpp"
#include "models/audio/FFTProcessor.hpp"

namespace nx
{
  class AudioChannelPipeline final : public ChannelPipeline
  {
  public:
    AudioChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : ChannelPipeline( ctx, channelId )
    {}

    ~AudioChannelPipeline() override = default;

    void processAudioBuffer( FFTBuffer& buffer )
    {
      // receive the unscaled audio buffer
      const auto& audioBuffer = buffer.getBuffer();

      // scale the FFT buffer to user-customizable values
      m_scaler.apply( m_ctx.globalInfo.sampleRate, audioBuffer );
      m_particleLayout.processAudioBuffer( m_scaler );
      m_shaderPipeline.processAudioBuffer( audioBuffer );
    }

    void update( const sf::Time& deltaTime ) const override
    {
      m_particleLayout.update( deltaTime );
      m_modifierPipeline.update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void drawMenu() override
    {
      ImGui::Text( "Particle count: %ld", m_particleLayout.getParticles().size() );
      ImGui::Separator();
      m_scaler.drawMenu();
      m_particleLayout.drawAudioMenu();

      ImGui::Separator();
      m_modifierPipeline.drawMenu();

      ImGui::Separator();
      m_shaderPipeline.drawMenu();

      ImGui::Separator();
      drawChannelPipelineMenu();
      ImGui::Separator();
    }

  private:

    FFTProcessor m_scaler;
    std::mutex m_audioVisualizerMutex;
  };

}