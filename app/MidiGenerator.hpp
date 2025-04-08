#pragma once


namespace nx::test
{
  class MidiGenerator
  {
    static inline int m_seed { 1 };

  public:

    MidiGenerator( int16_t channel, int32_t maxDelay )
      : m_rand( ++m_seed ),
        m_channel( channel ),
        m_maxDelay( maxDelay )
    {}

    bool isRunning() const { return m_isRunning; }

    void run( const std::function< void( const Steinberg::Vst::Event& ) >& onEvent )
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
            Steinberg::Vst::Event vstEvent;
            vstEvent.type = Steinberg::Vst::Event::kNoteOnEvent;
            vstEvent.noteOn.channel = m_channel;
            vstEvent.noteOn.pitch = m_rand() % 88 + 21;
            vstEvent.noteOn.velocity = std::max( ( m_rand() % 100 ) / 100.f, .5f );

            onEvent( vstEvent );

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
    bool m_isRunning { false };
    std::mt19937 m_rand;
    int16_t m_channel { 0 };
    std::unique_ptr< std::thread > m_thread;
    int32_t m_maxDelay { 0 };
  };

}
