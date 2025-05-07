#include "app/EventFacadeApp.hpp"

#include "log/Logger.hpp"

namespace nx
{

  EventFacadeApp::EventFacadeApp()
    : m_ctx( m_globalInfo, m_stateContext ),
      m_pipelines( m_ctx )
  {
    for ( int i = 1; i < m_midiGen.size(); ++i ) m_midiGen[ i ].toggleMute();
  }

  void EventFacadeApp::processVstEvent( const Steinberg::Vst::Event & event )
  {
    // forward it immediately. this should be as fast as
    // possible because this runs on the processor thread

    // WARNING:
    // comes in on the processor thread and not on the controller thread!
    if ( event.type != Steinberg::Vst::Event::kNoteOnEvent ) return;

    // push to a non-blocking queue
    m_queue.enqueue( {
      .channel = event.noteOn.channel,
      .pitch = event.noteOn.pitch,
      .velocity = event.noteOn.velocity
    } );
  }

  void EventFacadeApp::initialize( sf::RenderWindow & window )
  {
    LOG_INFO( "initializing event receiver" );
    if ( !ImGui::SFML::Init( window ) )
      LOG_ERROR( "initializing imgui failed" );

    m_globalInfo.windowSize = window.getSize();
    m_globalInfo.windowView.setSize( { static_cast< float >(m_globalInfo.windowSize.x), static_cast< float >(m_globalInfo.windowSize.y) } );
    m_globalInfo.windowHalfSize = m_globalInfo.windowView.getSize() / 2.f;
  }

  void EventFacadeApp::shutdown( const sf::RenderWindow & window )
  {
    LOG_INFO( "shutting down event receiver" );
    ImGui::SFML::Shutdown( window );

    for ( auto& midiGen : m_midiGen )
      midiGen.stop();

    m_pipelines.shutdown();
  }

  // gets called whenever the OS indicates it's time to update
  bool EventFacadeApp::executeFrame( sf::RenderWindow & window )
  {
    if ( !window.isOpen() ) return false;

    while ( const std::optional event = window.pollEvent() )
    {
      // post events
      ImGui::SFML::ProcessEvent( window, *event );
      // m_channelPipeline.processEvent( event );
      if ( event->getIf< sf::Event::Closed >() )
      {
        window.close();
        return false;
      }

      if ( const auto * resizeEvent = event->getIf< sf::Event::Resized >() )
        onResize( window, resizeEvent->size.x, resizeEvent->size.y );
      else if ( const auto * mouseEvent = event->getIf< sf::Event::MouseButtonReleased >() )
      {
        if ( mouseEvent->button == sf::Mouse::Button::Right )
          m_globalInfo.hideMenu = !m_globalInfo.hideMenu;
      }
    }

    // update
    {
      const auto delta = m_timer.restart();
      m_globalInfo.elapsedTimeSeconds += delta.asSeconds();
      ++m_globalInfo.frameCount;
      ImGui::SFML::Update( window, delta );
      consumeMidiEvents();
      m_pipelines.update( delta );
    }

    // draw
    {
      window.clear();

      // the problem seems to be part of this!
      m_pipelines.draw( window );

      if ( !m_globalInfo.hideMenu )
        drawMenu();

      ImGui::SFML::Render( window );

      // window.setView( m_windowInfo.windowView );
      window.display();
    }

    return true;
  }

  void EventFacadeApp::onResize( sf::RenderWindow & window, uint32_t width, uint32_t height )
  {
    m_globalInfo.windowSize = { width, height };
    m_globalInfo.windowView.setSize( { static_cast< float >(width),
                                            static_cast< float >(height) } );
    m_globalInfo.windowHalfSize = { static_cast< float >(width) / 2, static_cast< float >(height) / 2 };
  }

  void EventFacadeApp::drawMenu()
  {
    // draw ImGui last so it sits on top of the screen
    ImGui::Begin( "nxvfx", nullptr, ImGuiWindowFlags_AlwaysAutoResize );

    if ( m_midiGenIsRunning )
    {
      if ( ImGui::Button( "Stop MIDI" ) )
      {
        for ( auto& midiGen : m_midiGen )
          midiGen.stop();

        m_midiGenIsRunning = false;
      }
    }
    else
    {
      if ( ImGui::Button( "Start MIDI" ) )
      {
        for ( auto& midiGen : m_midiGen )
          midiGen.run();

        m_midiGenIsRunning = true;
      }
    }

    for ( auto& midiGen : m_midiGen )
    {
      ImGui::PushID( &midiGen );
      bool isMuted = midiGen.isMuted();
      if ( ImGui::Checkbox( "mute channel", &isMuted ) ) midiGen.toggleMute();
      ImGui::SameLine();
      if ( ImGui::Button( "send midi" ) )
      {
        midiGen.next();
      }
      ImGui::PopID();
    }

    ImGui::End();

    m_pipelines.drawMenu();
  }

  void EventFacadeApp::consumeMidiEvents()
  {
    // this occurs on the controller thread
    Midi_t midiEvent;
    while ( m_queue.try_dequeue( midiEvent ) )
      m_pipelines.processMidiEvent( midiEvent );
  }


}