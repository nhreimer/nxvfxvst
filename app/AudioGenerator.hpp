#pragma once

#include <cmath>
#include <imgui.h>

#include "helpers/MathHelper.hpp"
#include "vst/analysis/AudioAnalyzer.hpp"

namespace nx::test
{

  class AudioGenerator final
  {
  public:

    explicit AudioGenerator( const float frequency )
      : m_frequency( frequency )
    {}

    void drawMenu()
    {
      ImGui::PushID( this );

      if ( ImGui::TreeNode( "Audio Gen Options" ) )
      {
        if ( ImGui::Checkbox( "mute channel", &m_isMuted ) )
        {
          m_isMuted = !m_isMuted;
        }

        ImGui::SliderFloat( "Sample Rate##1", &m_sampleRate, 0.f, 96000.f );
        ImGui::SliderInt( "Buffer Size##1", &m_bufferSize, 1, 2048 );

        ImGui::Separator();
        ImGui::SliderFloat( "Frequency##1", &m_frequency, 20.f, 20000.f, "%0.2f", ImGuiSliderFlags_Logarithmic );
        ImGui::SliderFloat( "Phase##1", &m_phase, 0.f, 5.f );
        ImGui::SliderFloat( "Amplitude##1", &m_amplitude, 0.f, 10.f );
        ImGui::SliderFloat( "Spike Multiplier##1", &m_spikeMultiplier, 1.f, 20.f );
        ImGui::SliderFloat( "Spike Interval (seconds)##1", &m_spikeIntervalInSeconds, 0.f, 20.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::PopID();
    }

    float generateSample()
    {
      if ( m_isMuted ) return 0.f;

      const float sample = std::sin(m_phase * NX_TAU);

      // Advance phase using fixed frequency
      m_phase += m_frequency / m_sampleRate;
      if (m_phase >= 1.f)
        m_phase -= 1.f;

      if ( m_clock.getElapsedTime().asSeconds() >= m_spikeIntervalInSeconds )
      {
        m_clock.restart();
        return sample * m_amplitude * m_spikeMultiplier;
      }

      return sample * m_amplitude;
    }

  private:
    bool m_isMuted { false };
    int32_t m_bufferSize { 256 };
    float m_frequency = 440.f;  // A4
    float m_sampleRate = 44100.f;
    float m_phase = 0.f;
    float m_time = 0.f;
    float m_amplitude = 0.8f;
    float m_spikeMultiplier = 1.f;
    float m_spikeIntervalInSeconds = 3.f;

    sf::Clock m_clock;
  };

///////////////////////////////////////////////////////////////////////////////

  class AudioGeneratorMixer final
  {
  public:

    explicit AudioGeneratorMixer( const std::function< void( AudioDataBuffer& ) >& callback )
      : m_bufferReadyCallback( callback )
    {}

    ~AudioGeneratorMixer()
    {
      stop();
    }

    void drawMenu()
    {
      for ( auto& oscillator : m_oscillators )
      {
        ImGui::SeparatorText( "Audio Generator" );
        oscillator.drawMenu();
      }
    }

    bool isRunning() const { return m_isRunning; }

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
            std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
            continue;
          }

          float frequency = 0.f;

          for ( auto& oscillator : m_oscillators )
          {
            frequency += oscillator.generateSample();
          }

          frequency /= static_cast< float >( m_oscillators.size() );

          m_analyzer.pushSample( frequency );
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

    std::array< AudioGenerator, 3 > m_oscillators
    {
      // A Major
      AudioGenerator { 440.f },   // A4
      AudioGenerator { 554.37f }, // C#5
      AudioGenerator { 659.25f }  // E5
    };

    AudioAnalyzer m_analyzer;

    bool m_isMuted { false };
    bool m_isRunning { false };
    std::unique_ptr< std::thread > m_thread;
    std::function< void( AudioDataBuffer& ) > m_bufferReadyCallback;
  };

}