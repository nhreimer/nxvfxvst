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

#include <queue>

#include "models/channel/AudioChannelPipeline.hpp"
#include "models/channel/MidiChannelPipeline.hpp"
#include "data/PipelineContext.hpp"
#include "helpers/Definitions.hpp"
#include "models/encoder/EncoderFactory.hpp"
#include "shapes/TimedMessage.hpp"
#include "utils/ChannelWorker.hpp"
#include "utils/ImGuiFrameDiagnostics.hpp"

#ifdef BUILD_PLUGIN
#include "vst/version.h"
#else
#include "app/version.hpp"
#endif

namespace nx
{
  class FFTBuffer;

  class MultichannelPipeline final
  {

    struct ChannelDrawingData_t
    {
      int32_t priority { 0 };
      ChannelPipeline * channel { nullptr };
      ChannelWorker * channelWorker { nullptr };

      // Overload '<' for std::priority_queue (max-heap)
      // Lower priority value = higher actual priority
      bool operator<(const ChannelDrawingData_t& other) const
      {
        return priority > other.priority;
      }

      bool operator>(const ChannelDrawingData_t& other) const
      {
        return priority < other.priority;
      }
    };

  public:
    explicit MultichannelPipeline( PipelineContext& context );
    ~MultichannelPipeline() = default;

    [[nodiscard]]
    nlohmann::json saveState() const;
    void restoreState( const nlohmann::json &j ) const;

    void processMidiEvent( const Midi_t &midi ) const;
    void processAudioData( FFTBuffer& buffer );

    void draw(sf::RenderWindow &window);
    void drawMenu();

    void update(const sf::Time &deltaTime);

    void shutdown() const;

  private:

    void drawPipelineMenu();
    void drawPipelineMetrics();

  private:

    PipelineContext m_ctx;

    std::array< std::unique_ptr< ChannelPipeline >, MAX_CHANNELS > m_channels;
    std::array< std::unique_ptr< ChannelWorker >, MAX_CHANNELS > m_channelWorkers;

    TimedMessage m_messageClock;

    int m_selectedChannel { 0 };

    E_Encoder m_encoderType { E_Encoder::E_RawRGBA };
    EncoderData_t m_encoderData {};

    // used for saving to video
    std::unique_ptr< IEncoder > m_encoder;

    int32_t m_selectedCodec = 0;

    std::priority_queue< ChannelDrawingData_t > m_drawingPrioritizer;

    ImGuiFrameDiagnostics m_frameDiagnostics;
    RingBufferAverager m_totalRenderAverage { RENDER_SAMPLES_COUNT };

    float m_mainWindowOpacity { 0.5f };
    float m_metricsWindowOpacity { 0.3f };

    RingBufferAverager m_audioDataAverage { RENDER_SAMPLES_COUNT };

    static constexpr int32_t AUDIO_CHANNEL_INDEX = 0;
    static constexpr int32_t MIDI_CHANNEL_INDEX = 1;
  };

} // namespace nx
