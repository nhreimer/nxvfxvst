#pragma once

namespace nx
{
  struct ProfileMetrics
  {
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;

    double elapsedMilliseconds() const
    {
      return std::chrono::duration< double, std::milli >(endTime - startTime).count();
    }
  };
}