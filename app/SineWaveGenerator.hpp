#pragma once
#include <cmath>
#include <imgui.h>
#include "helpers/MathHelper.hpp"
#include "vst/analysis/AudioAnalyzer.hpp"

namespace nx::test
{

  class SineSweepGenerator
  {
  public:

    explicit SineSweepGenerator( const std::function< void( AudioDataBuffer& ) >& callback )
      : m_bufferReadyCallback( callback )
    {}

    ~SineSweepGenerator()
    {
      stop();
    }

    bool isRunning() const { return m_isRunning; }

    bool isMuted() const { return m_isMuted; }
    void toggleMute() { m_isMuted = !m_isMuted; }

    void reset()
    {
      m_phase = 0.f;
      m_time = 0.f;
    }

    void drawMenu()
    {
      ImGui::PushID( this );

      if ( ImGui::TreeNode( "Audio Gen Options" ) )
      {
        if ( ImGui::Checkbox( "mute channel", &m_isMuted ) ) toggleMute();

        ImGui::SliderFloat( "Sample Rate##1", &m_sampleRate, 0.f, 96000.f );
        ImGui::SliderInt( "Buffer Size##1", &m_bufferSize, 1, 2048 );

        ImGui::Separator();
        // ImGui::SliderFloat( "Freq Start##1", &m_startFreq, 0.f, m_endFreq );
        // ImGui::SliderFloat( "Freq End##1", &m_endFreq, m_startFreq, 20000.f );
        ImGui::SliderFloat( "Frequency##1", &m_frequency, 0.f, 20000.f );

        // ImGui::SliderFloat( "Duration (secs)##1", &m_duration, 0.f, 20.f );
        ImGui::SliderFloat( "Phase##1", &m_phase, 0.f, 5.f );
        ImGui::SliderFloat( "Amplitude##1", &m_amplitude, 0.f, 10.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::PopID();
    }

    void run()
    {
      if ( m_thread )
      {
        LOG_WARN( "audio generator already running" );
        return;
      }

      m_isRunning = true;

      m_thread = std::make_unique< std::thread >(
      [&]()
      {
        LOG_INFO( "Starting audio generator" );
        while ( m_isRunning )
        {
          if ( m_isMuted )
          {
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
            continue;
          }

          for ( int i = 0; i < m_bufferSize; ++i )
            m_analyzer.pushSample( generateSample() );

          if ( m_analyzer.isFFTReady() )
          {
            AudioDataBuffer buffer;
            m_analyzer.computeFFT( buffer );
            m_bufferReadyCallback( buffer );

            // pause similarly to the way the processor works, which is ~1/60
            std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
          }
        }
      });
    }

    void stop()
    {
      if ( !m_thread )
      {
        LOG_INFO( "audio generator already stopped!" );
        return;
      }

      m_isRunning = false;
      LOG_INFO( "shutting down thread..." );
      m_thread->join();
      LOG_INFO( "thread exited." );
      m_thread.reset();
    }

  private:

    // Call this each audio callback or simulation step
    // float generateSample()
    // {
    //   if (m_time > m_duration)
    //     return 0.f;
    //
    //   // Linearly interpolate frequency over time
    //   const float t = m_time / m_duration;
    //   const float freq = std::lerp( m_startFreq, m_endFreq, t );
    //
    //   const float sample = std::sin(m_phase * NX_TAU);
    //
    //   // Phase increment
    //   m_phase += freq / m_sampleRate;
    //   if (m_phase >= 1.f)
    //     m_phase -= 1.f;
    //
    //   // Advance time
    //   m_time += 1.f / m_sampleRate;
    //
    //   return sample * m_amplitude;
    // }

    float generateSample()
    {
      const float sample = std::sin(m_phase * NX_TAU);

      // Advance phase using fixed frequency
      m_phase += m_frequency / m_sampleRate;
      if (m_phase >= 1.f)
        m_phase -= 1.f;

      return sample * m_amplitude;
    }

  private:

    bool m_isMuted { false };
    bool m_isRunning { false };

    int32_t m_bufferSize { 256 };
    float m_frequency = 440.f;  // A4
    // float m_startFreq = 20.f;
    // float m_endFreq = 20000.f;
    // float m_duration = 5.f; // sweep duration in seconds
    float m_sampleRate = 44100.f;
    float m_phase = 0.f;
    float m_time = 0.f;
    float m_amplitude = 0.8f;

    std::unique_ptr< std::thread > m_thread;
    AudioAnalyzer m_analyzer;

    std::function< void( AudioDataBuffer& ) > m_bufferReadyCallback;
  };

}