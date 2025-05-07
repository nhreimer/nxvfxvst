#pragma once

#include <vector>
#include <chrono>

#include "log/Logger.hpp"

namespace nx
{
  // this assumes fixed intervals. it's possible that a time-weighted
  // one will give a more accurate picture.
  class RingBufferAverager final
  {
  public:

    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double>; // seconds
    using TimePoint = Clock::time_point;

    explicit RingBufferAverager(const size_t capacity)
      : m_buffer(capacity, 0.0)
    {}

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
      }
    }

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
  };

}