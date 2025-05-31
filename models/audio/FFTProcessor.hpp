#pragma once

#include "models/audio/IFFTResult.hpp"
#include "helpers/Definitions.hpp"

namespace nx
{

  /// scales and smooths an FFT buffer from the Processor VST3 side.
  class FFTProcessor final : public IFFTResult
  {
    enum class Mode
    {
      Linear,
      Decibel
    };

    struct FFTScalerData_t
    {
      Mode mode = Mode::Decibel;
      float gain = 1.f;
      float dbFloor = -60.f;
      float curvePower = 0.25f;
      bool clamp = true;

      float decayRate = 3.5f;
    };

  public:

    // TODO: maybe move to AudioHelper to distinguish its role?
    static float getLogFrequencyForBin( const size_t binIndex,
                                        const float sampleRate,
                                        const size_t totalBins )
    {
      const float maxFreq = sampleRate / 2.f;

      const float t = static_cast< float >(binIndex) / static_cast<float>(totalBins - 1);
      return MIN_FREQ * std::pow( maxFreq / MIN_FREQ, t );
    }

    const AudioDataBuffer& getSmoothedBuffer() const override { return m_smoothedBins; }
    const AudioDataBuffer& getRealTimeBuffer() const override { return m_realTimeBins; }

    // const AudioDataBuffer& getLogSmoothedBuffer() const override { return m_logSmoothedBins; }
    // const AudioDataBuffer& getLogRealTimeBuffer() const override { return m_logRealTimeBins; }

    void apply( const float sampleRate, const AudioDataBuffer& bins )
    {
      if ( bins.size() != FFT_BINS )
      {
        LOG_ERROR( "FFT bin size mismatch. unable to process." );
        return;
      }

      for (size_t i = 0; i < FFT_BINS; ++i)
      {
        const float scaled = scale(bins[i]);

        float& current = m_smoothedBins[i];
        if (scaled > current)
        {
          current = scaled;
        }
        else
        {
          const auto decayAmount = m_data.decayRate * ImGui::GetIO().DeltaTime;
          current = std::max(current - decayAmount, scaled);
        }

        m_realTimeBins[i] = scaled;
      }

      // mapToLogScale( sampleRate );
    }

    void drawMenu()
    {
      if ( ImGui::TreeNode( "FFT Options" ) )
      {
        if ( ImGui::RadioButton( "Linear", m_data.mode == Mode::Linear ) )
          m_data.mode = Mode::Linear;
        else if ( ImGui::RadioButton( "Decibel", m_data.mode == Mode::Decibel ) )
          m_data.mode = Mode::Decibel;

        ImGui::SliderFloat("Gain##1", &m_data.gain, 0.1f, 20.f, "%.2fx");
        ImGui::SliderFloat("dB Floor##1", &m_data.dbFloor, -100.f, -20.f);
        ImGui::SliderFloat("Curve Power", &m_data.curvePower, 0.1f, 1.5f, "%.2f");
        ImGui::SliderFloat( "Decay Rate##1", &m_data.decayRate, 0.1f, 20.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    float scale(const float raw) const
    {
      float val = std::max(raw * m_data.gain, 1e-8f);

      if (m_data.mode == Mode::Decibel)
      {
        const float db = MIN_FREQ * std::log10(val);
        val = (db - m_data.dbFloor) / std::abs(m_data.dbFloor);
      }

      val = std::pow(val, m_data.curvePower);

      if (m_data.clamp)
        val = std::clamp(val, 0.f, 1.f);

      return val;
    }

    // void mapToLogScale( const float sampleRate )
    // {
    //   const float nyquist = sampleRate / 2.f;
    //   //constexpr float minFreq = 20.f;
    //   const float logMin = std::log10(MIN_FREQ);
    //   const float logMax = std::log10(nyquist);
    //   const float logRange = logMax - logMin;
    //
    //   for (size_t i = 0; i < FFT_BINS; ++i)
    //   {
    //     const float freq = (i / static_cast<float>(FFT_BINS)) * nyquist;
    //     if (freq < MIN_FREQ)
    //     {
    //       m_logRealTimeBins[i] = 0.f;
    //       m_logSmoothedBins[i] = 0.f;
    //       continue;
    //     }
    //
    //     // Find the original bin index from log freq
    //     const float logFreq = std::log10(freq);
    //     const float binFloat = (logFreq - logMin) / logRange * FFT_BINS;
    //     const int32_t srcIndex = std::clamp(
    //       static_cast<int32_t>(binFloat),
    //       0,
    //       static_cast< int32_t >(FFT_BINS) - 1);
    //
    //     m_logRealTimeBins[i] = m_realTimeBins[srcIndex];
    //     m_logSmoothedBins[i] = m_smoothedBins[srcIndex];
    //   }
    // }

  private:
    FFTScalerData_t m_data;

    // these are linear bins
    AudioDataBuffer m_realTimeBins {}; // semi-real-time buffer
    AudioDataBuffer m_smoothedBins {}; // historical data for smoothing

    AudioDataBuffer m_logRealTimeBins {};
    AudioDataBuffer m_logSmoothedBins {};

  };

}