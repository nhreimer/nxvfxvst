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


#if defined WIN32
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
#elif defined __linux__

#include <chrono>
namespace nx
{
  class FrameRateLock final
  {
  public:
    explicit FrameRateLock( const float fps = 60.f )
    {
      m_targetDelta = 1.0 / fps; // x FPS, e.g., 60 FPS
      m_accumulatedTime = 0.0;
      ::clock_gettime( CLOCK_MONOTONIC, &m_lastTime );
    }

    // Call once per frame; returns true when it's time to render a new frame
    bool shouldRenderFrame()
    {
      timespec now;
      ::clock_gettime( CLOCK_MONOTONIC, &now);

      const double delta = ( now.tv_sec - m_lastTime.tv_sec ) +
                           ( now.tv_nsec - m_lastTime.tv_nsec ) / 1e9;
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
    timespec m_lastTime;
    double m_targetDelta;
    double m_accumulatedTime;
  };
}
#endif