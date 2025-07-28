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

#include <deque>
#include <chrono>

namespace nx
{
  // this isn't very efficient because of std::deque, but it's good
  // for looking at averages within a rolling window
  // for performance metrics, see RingBufferAverage
  class TimedAverager
  {
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;

    explicit TimedAverager( const Duration windowDuration )
      : m_windowDuration( windowDuration ),
        m_total( 0.f )
    {}

    Duration getWindowDuration() const { return m_windowDuration; }

    void addValue( const double value, const TimePoint timestamp = Clock::now() )
    {
      m_values.emplace_back(timestamp, value);
      m_total += value;
      trimOldValues(timestamp);
    }

    double getAverage() const
    {
      if (m_values.empty()) return 0.f;
      return m_total / static_cast<double>(m_values.size());
    }

    /// Returns how many seconds the oldest value exceeded the time window
    double getOverflowOffset() const
    {
      if (m_values.empty())
        return 0.0;

      const auto now = Clock::now();
      const auto oldestTime = m_values.front().first;
      const auto timeDelta = now - oldestTime;

      const auto overflow = Duration(timeDelta) - m_windowDuration;
      return overflow.count() > 0.0 ? overflow.count() : 0.0;
    }

    size_t getCount() const
    {
      return m_values.size();
    }

  private:
    void trimOldValues(const TimePoint now)
    {
      while (!m_values.empty())
      {
        const auto& [ timestamp, value ] = m_values.front();
        if ( (now - timestamp) > m_windowDuration )
        {
          m_total -= value;
          m_values.pop_front();
        }
        else
          break;
      }
    }

    Duration m_windowDuration;
    std::deque<std::pair<TimePoint, double>> m_values;
    double m_total;
  };
}