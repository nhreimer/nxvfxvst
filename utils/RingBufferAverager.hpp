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

#include <vector>
#include <chrono>

#include "log/Logger.hpp"

namespace nx
{
  // this assumes fixed intervals. it's possible that a time-weighted
  // one will give a more accurate picture, but this is most likely good enough
  class RingBufferAverager final
  {
  public:

    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double>; // seconds
    using TimePoint = Clock::time_point;

    explicit RingBufferAverager(const size_t capacity = RENDER_SAMPLES_COUNT)
      : m_buffer(capacity, 0.0)
    {
      m_cycleTime = Clock::now();
    }

    void addSample(const double value)
    {
      double& current = m_buffer[m_position];
      m_sum -= current;
      m_sum += value;
      current = value;

      if (++m_position == static_cast<int32_t>(m_buffer.size()))
      {
        m_position = 0;
        if (!m_isBufferFull)
        {
          m_isBufferFull = true;
          LOG_DEBUG("buffer ring full: {}", m_buffer.size());
        }

        const auto now = Clock::now();
        const Duration duration = now - m_cycleTime;
        m_cycleTime = now;
        m_cycleDurationInMs = duration.count() * 1000.0;
      }
    }

    double getCycleTimeInMs() const { return m_cycleDurationInMs; }

    void startTimer()
    {
      if ( m_activeTime != EMPTY_TIME )
      {
        LOG_WARN( "startTimer() called before stopTimer()" );
      }

      m_activeTime = Clock::now();
    }

    void stopTimerAndAddSample()
    {
      if ( m_activeTime == EMPTY_TIME )
      {
        LOG_ERROR( "Called stopTimer() before calling startTimer()" );
        return;
      }

      const TimePoint currentTime = Clock::now();
      const Duration elapsed = currentTime - m_activeTime;
      const double durationInSeconds = elapsed.count();

      m_activeTime = EMPTY_TIME;

      addSample( durationInSeconds * 1000.f );
    }

    void reset()
    {
      m_buffer.clear();
      m_isBufferFull = false;
      m_position = 0;
      m_sum = 0.f;
    }

    double getAverage() const
    {
      if (m_sum == 0.0)
        return 0.0;

      const int32_t count = getSampleCount();
      if (count == 0) return 0.0;

      return m_sum / static_cast<double>(count);
    }

    int32_t getSampleCount() const
    {
      return m_isBufferFull
        ? static_cast<int32_t>(m_buffer.size())
        : m_position;
    }

  private:
    std::vector<double> m_buffer;
    int32_t m_position { 0 };
    double m_sum { 0.0 };
    bool m_isBufferFull { false };

    TimePoint m_activeTime { EMPTY_TIME };

    static constexpr TimePoint EMPTY_TIME {};

    TimePoint m_cycleTime {};
    double m_cycleDurationInMs { 0.0 };
  };

}