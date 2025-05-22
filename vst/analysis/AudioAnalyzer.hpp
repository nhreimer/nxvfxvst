#pragma once

#include <array>
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <cmath>

#include "helpers/Definitions.hpp"

namespace nx
{

  class AudioAnalyzer final
  {

  public:

    AudioAnalyzer()
    {
      m_fft_cfg = kiss_fftr_alloc( FFT_SIZE, 0, nullptr, nullptr );
    }

    ~AudioAnalyzer()
    {
      if ( m_fft_cfg )
        free( m_fft_cfg );
    }

    void pushSample( const float sample )
    {
      m_sampleBuffer[ m_sampleWriteIndex ] = sample;

      ++m_sampleWriteIndex;

      if ( m_sampleWriteIndex >= FFT_SIZE )
      {
        m_ready = true;
        m_sampleWriteIndex = 0;
      }
    }

    bool isFFTReady() const { return m_ready; }

    void computeFFT( std::array< float, FFT_BINS >& outBins )
    {
      // not ready, ignore request
      if ( !m_ready ) return;

      // reset it
      m_ready = false;

      // std::array< kiss_fft_cpx, FFT_BINS > fftOut;
      kiss_fftr( m_fft_cfg, m_sampleBuffer.data(), m_fftOut.data() );

      for ( size_t i = 0; i < FFT_BINS; ++i )
      {
        const float mag = std::sqrt( m_fftOut[ i ].r * m_fftOut[ i ].r +
                                     m_fftOut[ i ].i * m_fftOut[ i ].i);

        outBins[ i ] = mag / ( FFT_SIZE * 0.5f ); // normalize
      }
    }

  private:
    std::array< kiss_fft_cpx, FFT_BINS > m_fftOut {};
    std::array< float, FFT_SIZE > m_sampleBuffer {};
    size_t m_sampleWriteIndex { 0 };
    bool m_ready { false };

    kiss_fftr_cfg m_fft_cfg { nullptr };
  };

}