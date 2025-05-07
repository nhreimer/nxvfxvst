#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "utils/TimedAverager.hpp"
#include "utils/ProfileMetics.hpp"

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
    const TimedAverager& getMetrics()
    {
      std::lock_guard lock(m_metricsMutex);
      return m_timedAverager;
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

        ProfileMetrics metrics;
        metrics.startTime = std::chrono::steady_clock::now();

        if (m_pipelineFunc)
          m_pipelineFunc();

        metrics.endTime = std::chrono::steady_clock::now();

        {
          std::lock_guard metricsLock(m_metricsMutex);
          m_timedAverager.addValue( metrics.elapsedMilliseconds() );
        }

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
    //ProfileMetrics m_lastMetrics;
    TimedAverager m_timedAverager { std::chrono::seconds( RENDER_AVERAGE_SECONDS ) };
  };


} // namespace nx
