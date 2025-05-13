#pragma once

#include <random>

namespace nx::test
{
  class MidiGenerator
  {
    static inline int m_seed { 1 };

  public:

    MidiGenerator( const int16_t channel,
                   const int32_t maxDelay,
                   const std::function< void( const Steinberg::Vst::Event& ) >& onEvent )
      : m_rand( ++m_seed ),
        m_channel( channel ),
        m_maxDelay( maxDelay ),
        m_onEvent( onEvent )
    {}

    bool isRunning() const { return m_isRunning; }

    bool isMuted() const { return m_isMuted; }
    void toggleMute() { m_isMuted = !m_isMuted; }

    void drawMenu()
    {
      ImGui::PushID( this );

      if ( ImGui::Checkbox( "mute channel", &m_isMuted ) ) toggleMute();
      ImGui::SameLine();
      if ( ImGui::Button( "send midi" ) ) next();

      ImGui::SetNextItemWidth( 186.f );
      ImGui::SliderInt( "##Max Delay", &m_maxDelay, 0, 5000, "Delay %d" );

      ImGui::PopID();
    }

    void next()
    {
      if ( !m_isRunning )
      {
        Steinberg::Vst::Event vstEvent;
        vstEvent.type = Steinberg::Vst::Event::kNoteOnEvent;
        vstEvent.noteOn.channel = m_channel;
        vstEvent.noteOn.pitch = m_rand() % 88 + 21;
        vstEvent.noteOn.velocity = 127.f; //std::max( ( m_rand() % 100 ) / 100.f, .5f );
        m_onEvent( vstEvent );
      }
    }

    void run()
    {
      if ( m_thread )
      {
        LOG_WARN( "midi generator already running" );
        return;
      }

      m_isRunning = true;

      m_thread = std::make_unique< std::thread >(
        [&]()
        {
          while ( m_isRunning )
          {
            if ( m_isMuted )
            {
              std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
              continue;
            }

            Steinberg::Vst::Event vstEvent;
            vstEvent.type = Steinberg::Vst::Event::kNoteOnEvent;
            vstEvent.noteOn.channel = m_channel;
            vstEvent.noteOn.pitch = m_rand() % 88 + 21;
            vstEvent.noteOn.velocity = std::max( ( m_rand() % 100 ) / 100.f, .5f );

            m_onEvent( vstEvent );

            std::this_thread::sleep_for(
              std::chrono::milliseconds( m_rand() % m_maxDelay ) );
          }
          LOG_DEBUG( "thread exiting..." );
        });
    }

    void stop()
    {
      if ( !m_thread )
      {
        LOG_WARN( "midi generator already stopped" );
        return;
      }

      m_isRunning = false;
      LOG_INFO( "shutting down thread..." );
      m_thread->join();
      LOG_INFO( "thread exited." );
      m_thread.reset();
    }

  private:
    const std::function< void( const Steinberg::Vst::Event& ) > m_onEvent;
    bool m_isMuted { false };
    bool m_isRunning { false };
    std::mt19937 m_rand;
    int16_t m_channel { 0 };
    std::unique_ptr< std::thread > m_thread;
    int32_t m_maxDelay { 0 };
  };

}
