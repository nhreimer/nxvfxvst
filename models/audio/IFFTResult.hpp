#pragma once

namespace nx
{

  struct IFFTResult
  {
    virtual ~IFFTResult() = default;
    virtual const AudioDataBuffer& getSmoothedBuffer() const = 0;
    virtual const AudioDataBuffer& getRealTimeBuffer() const = 0;
  };

}