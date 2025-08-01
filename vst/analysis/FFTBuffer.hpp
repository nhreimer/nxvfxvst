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

#include <array>
#include <atomic>

#include "pluginterfaces/vst/ivstattributes.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include "helpers/Definitions.hpp"

namespace nx
{

  ///
  /// Double-buffer for easy production and consumption.
  /// it also keeps track of staleness in case drift happens.
  class FFTBuffer final
  {
  public:

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    [[nodiscard]]
    AudioDataBuffer& getBuffer()
    {
      if ( m_hasNewData.exchange( false, std::memory_order_acq_rel ) )
        std::swap(m_frontBuffer, m_backBuffer);

      return m_frontBuffer;
    }

    /// Unpacks the VST message and writes to the internal buffer
    /// @param message
    void write( Steinberg::Vst::IMessage * message )
    {
      if ( !message || !message->getAttributes() ) return;

      const void * rawPtr = nullptr;
      Steinberg::uint32 rawSize = 0;

      if (message->getAttributes()->getBinary("FFTData", rawPtr, rawSize) != Steinberg::kResultOk)
      {
        LOG_ERROR("unable to decode FFTData message");
        return;
      }

      if (rawSize != sizeof(float) * m_backBuffer.size())
      {
        LOG_ERROR("FFTData message has wrong size. got {}, expected {}",
                  rawSize, m_backBuffer.size() * sizeof(float));
        return;
      }

      std::memcpy( m_backBuffer.data(), rawPtr, rawSize );
      m_lastWrite = Clock::now();
      m_hasNewData.store(true, std::memory_order_release);
    }

    /// copies the Audio data from the processor over
    /// @param buffer the audio buffer directly from the AudioAnalyzer
    void write( const AudioDataBuffer & buffer )
    {
      std::memcpy( m_backBuffer.data(), buffer.data(), buffer.size() * sizeof( float ) );
      m_lastWrite = Clock::now();
      m_hasNewData.store(true, std::memory_order_release);
    }

    [[nodiscard]]
    std::chrono::milliseconds getAge() const
    {
      return std::chrono::duration_cast< std::chrono::milliseconds >( Clock::now()
             - m_lastWrite );
    }

    [[nodiscard]]
    bool isStale( const std::chrono::milliseconds threshold ) const
    {
      return getAge() > threshold;
    }

  private:
    std::atomic< bool > m_hasNewData { false };
    AudioDataBuffer m_frontBuffer {};
    AudioDataBuffer m_backBuffer {};

    TimePoint m_lastWrite { Clock::now() };
  };


}