#pragma once

#include "models/audio/IFFTResult.hpp"
#include "helpers/Definitions.hpp"

#include "utils/RingBufferAverager.hpp"

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
      Mode mode = Mode::Linear;
      float gain = 1.f;
      float dbFloor = -60.f;
      float curvePower = 0.25f;
      bool clamp = true;

      float decayRate = 3.5f;
    };

  public:

    [[nodiscard]]
    const AudioDataBuffer& getSmoothedBuffer() const override { return m_smoothedBins; }

    [[nodiscard]]
    const AudioDataBuffer& getRealTimeBuffer() const override { return m_realTimeBins; }

    void apply( const float sampleRate, const AudioDataBuffer& bins )
    {
      m_timedBuffer.startTimer();

      if ( bins.size() != FFT_BINS )
      {
        LOG_ERROR( "FFT bin size mismatch. unable to process." );
        m_timedBuffer.stopTimerAndAddSample();
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

      m_timedBuffer.stopTimerAndAddSample();
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

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    [[nodiscard]]
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

  private:
    FFTScalerData_t m_data;

    // these are linear bins
    AudioDataBuffer m_realTimeBins {}; // semi-real-time buffer
    AudioDataBuffer m_smoothedBins {}; // historical data for smoothing

    RingBufferAverager m_timedBuffer;
  };

}