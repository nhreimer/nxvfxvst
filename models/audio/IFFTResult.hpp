#pragma once

namespace nx
{

  struct IFFTResult
  {
    virtual ~IFFTResult() = default;
    virtual const AudioDataBuffer& getSmoothedBuffer() const = 0;
    virtual const AudioDataBuffer& getRealTimeBuffer() const = 0;

    virtual const AudioDataBuffer& getLogSmoothedBuffer() const = 0;
    virtual const AudioDataBuffer& getLogRealTimeBuffer() const = 0;

    // Optional: Musical or note-mapped buffers?
    // virtual const std::vector<NoteEnergy>& getNoteMappedBuffer() const = 0;
  };

}