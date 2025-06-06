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

    // TODO:
    // there are some common concerns that this doesn't take into account yet:
    // 1. harmonics picked up by the FFT, e.g., 110Hz spikes at 220, 330, etc.
    // 2. higher bins tend to be quieter. might need to scale bins accordingly
    // 3.


  public:

    AudioAnalyzer()
    {
      m_fft_cfg = kiss_fftr_alloc(
        FFT_SIZE,
        0,
        nullptr,
        nullptr );
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

      // this attempts to answer:
      // how much energy (amplitude) is present in the frequency band around bin i?
      // if you play a pure sine wave at 440 Hz, then only ONE of our bins will spike
      // while the rest will be near zero
      //
      // ______/\_______________  <-- an amazing graphical representation

      // this library does all the heavy lifting for us though
      // by running a real-to-complex FFT
      kiss_fftr( m_fft_cfg, m_sampleBuffer.data(), m_fftOut.data() );

      for ( size_t i = 0; i < FFT_BINS; ++i )
      {
        // convert the complex of the output to magnitude
        // the magnitude reflects the amplitude (NOT the phase, which is ignored)
        // the higher the energy at this frequency, the higher the bin value
        // fftBins[i] = sqrt(real^2 + imag^2)
        const float mag = std::sqrt( m_fftOut[ i ].r * m_fftOut[ i ].r +
                                        m_fftOut[ i ].i * m_fftOut[ i ].i );

        // normalize for consistent ranges
        outBins[ i ] = mag / ( FFT_SIZE * 0.5f );
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