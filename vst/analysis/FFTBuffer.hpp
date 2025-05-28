#pragma once

#include <array>

#include "pluginterfaces/vst/ivstattributes.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include "helpers/Definitions.hpp"

namespace nx
{

  ///
  /// Double-buffer for easy production and consumption.
  /// it also keeps track of staleness in case drift happens.
  class FFTBuffer final
  {
  public:

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    [[nodiscard]]
    AudioDataBuffer& getBuffer()
    {
      if ( m_hasNewData.exchange( false, std::memory_order_acq_rel ) )
        std::swap(m_frontBuffer, m_backBuffer);

      return m_frontBuffer;
    }

    void write( Steinberg::Vst::IMessage * message )
    {
      if ( !message || !message->getAttributes() ) return;

      const void * rawPtr = nullptr;
      Steinberg::uint32 rawSize = 0;

      if (message->getAttributes()->getBinary("FFTData", rawPtr, rawSize) != Steinberg::kResultOk)
      {
        LOG_ERROR("unable to decode FFTData message");
        return;
      }

      if (rawSize != sizeof(float) * m_backBuffer.size())
      {
        LOG_ERROR("FFTData message has wrong size. got {}, expected {}",
                  rawSize, m_backBuffer.size() * sizeof(float));
        return;
      }

      std::memcpy( m_backBuffer.data(), rawPtr, rawSize );
      m_lastWrite = Clock::now();
      m_hasNewData.store(true, std::memory_order_release);
    }

    [[nodiscard]]
    std::chrono::milliseconds age() const
    {
      return std::chrono::duration_cast< std::chrono::milliseconds >( Clock::now() - m_lastWrite );
    }

    [[nodiscard]]
    bool isStale( const std::chrono::milliseconds threshold ) const
    {
      return age() > threshold;
    }

  private:
    std::atomic< bool > m_hasNewData { false };
    AudioDataBuffer m_frontBuffer {};
    AudioDataBuffer m_backBuffer {};

    TimePoint m_lastWrite { Clock::now() };
  };


}