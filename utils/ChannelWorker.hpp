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

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "utils/RingBufferAverager.hpp"

#include "helpers/Definitions.hpp"

namespace nx
{

  class ChannelWorker
  {
  public:
    using PipelineFn = std::function< void() >;

    explicit ChannelWorker(PipelineFn pipelineFunc) : m_pipelineFunc(std::move(pipelineFunc))
    {
      m_thread = std::thread([ this ]() { threadLoop(); });
    }

    ~ChannelWorker()
    {
      {
        std::lock_guard lock(m_mutex);
        m_shouldExit = true;
        m_triggered = true;
      }

      LOG_INFO( "shutting down all tasks in channel" );

      m_cv.notify_one();
      if (m_thread.joinable())
        m_thread.join();
    }

    // Triggers the render thread to run the pipeline once
    void requestPipelineRun()
    {
      {
        std::lock_guard lock(m_mutex);
        m_triggered = true;
        m_pipelineComplete = false;
      }
      m_cv.notify_one();
    }

    // Blocks until the pipeline thread completes the current run
    void waitUntilComplete()
    {
      std::unique_lock lock(m_mutex);
      m_pipelineCompleteCv.wait(lock, [ this ] { return m_pipelineComplete; });
    }

    // Returns the latest profiling metrics
    double getMetrics() const
    {
      //std::lock_guard lock( m_metricsMutex );
      return m_averager.getAverage();
    }

  private:
    void threadLoop()
    {
      while (true)
      {

        std::unique_lock lock(m_mutex);
        m_cv.wait(lock, [ this ] { return m_triggered || m_shouldExit; });

        if (m_shouldExit)
          return;

        m_triggered = false;
        lock.unlock();

        m_averager.startTimer();

        if (m_pipelineFunc)
          m_pipelineFunc();

        m_averager.stopTimerAndAddSample();

        {
          std::lock_guard completeLock(m_mutex);
          m_pipelineComplete = true;
        }
        m_pipelineCompleteCv.notify_one();
      }
    }

    PipelineFn m_pipelineFunc;
    std::thread m_thread;
    std::condition_variable m_cv;
    std::condition_variable m_pipelineCompleteCv;
    std::mutex m_mutex;
    std::mutex m_metricsMutex;

    bool m_triggered = false;
    bool m_shouldExit = false;
    bool m_pipelineComplete = true;

    std::vector< std::function< void() > > m_cleanupQueue;
    RingBufferAverager m_averager { RENDER_SAMPLES_COUNT };
  };


} // namespace nx
