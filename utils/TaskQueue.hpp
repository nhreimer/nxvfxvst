#pragma once
#include <functional>
#include <concurrentqueue/blockingconcurrentqueue.h>

namespace nx
{

  ///////////////////////////////////////////////////////////
  struct RequestSink
  {
    virtual ~RequestSink() = default;

    // we don’t need the interface to know about std::function
    template <typename F>
    void request(F&& fn)
    {
      // perfect forwarding
      requestImpl(std::function<void()>(std::forward<F>(fn)));
    }

  protected:
    // override this and feed it to the internal TaskQueue or whatever implementation
    // we decide to use under the hood
    virtual void requestImpl(std::function<void()> fn) = 0;
  };

  ///////////////////////////////////////////////////////////
  class TaskQueue final
  {
  public:
    using TaskFn = std::function<void()>;

    // Delay type-erasure as long as possible.
    // Uses a templated pushTask() that wraps the lambda once into
    // a std::function<void()> right before enqueue
    template <typename F>
    void pushTask(F&& task)
    {
      static_assert(std::is_invocable_v<F>, "Task must be callable with no arguments");
      m_queue.enqueue(TaskFn(std::forward<F>(task)));
    }

    void runTasks()
    {
      TaskFn task;
      while (m_queue.try_dequeue(task))
        task();
    }

  private:
    moodycamel::BlockingConcurrentQueue<TaskFn> m_queue;
  };

  ///////////////////////////////////////////////////////////
  class TaskQueueRequestSink : public RequestSink
  {

  public:

    virtual void requestRenderUpdate() = 0;
    virtual void requestShutdown() = 0;

    virtual void runTasks()
    {
      m_taskQueue.runTasks();
    }

  protected:
    void requestImpl(std::function<void()> fn) override
    {
      m_taskQueue.pushTask(std::move(fn));
    }

  private:
    TaskQueue m_taskQueue;
  };

}
