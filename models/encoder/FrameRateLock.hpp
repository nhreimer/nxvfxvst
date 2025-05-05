#pragma once

#include <windows.h>

namespace nx
{
  /// this class helps prevent drift as much as possible on Windows
  /// because the Timer callback is variable. it's still not perfect
  /// but it should be good enough in most cases to get us close to 60 FPS
  class FrameRateLock final
  {
  public:
    explicit FrameRateLock( const float fps = 60.f )
    {
      ::QueryPerformanceFrequency(&m_frequency);
      ::QueryPerformanceCounter(&m_lastTime);
      m_targetDelta = 1.0 / fps; // x FPS, e.g., 60 FPS
      m_accumulatedTime = 0.0;
    }

    // Call once per frame; returns true when it's time to render a new frame
    bool shouldRenderFrame()
    {
      ::LARGE_INTEGER now;
      ::QueryPerformanceCounter(&now);

      const double delta = static_cast<double>(now.QuadPart - m_lastTime.QuadPart) /
                           m_frequency.QuadPart;
      m_lastTime = now;
      m_accumulatedTime += delta;

      if (m_accumulatedTime >= m_targetDelta)
      {
        m_accumulatedTime -= m_targetDelta;
        return true;
      }

      return false;
    }

  private:
    ::LARGE_INTEGER m_frequency;
    ::LARGE_INTEGER m_lastTime;
    double m_targetDelta;
    double m_accumulatedTime;
  };


}