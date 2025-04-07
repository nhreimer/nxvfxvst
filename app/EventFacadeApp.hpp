#pragma once

#include <concurrent_queue.h>
#include <deque>
#include <pluginterfaces/vst/ivstevents.h>

#include "log/Logger.hpp"

#include "models/data/GlobalInfo_t.hpp"
#include "models/ChannelPipeline.hpp"

#include "MidiGenerator.hpp"

namespace nx
{

  // Event Facade for the standalone, i.e., non-plugin version
  class EventFacadeApp final
  {
  public:

    void processVstEvent( const Steinberg::Vst::Event & event )
    {
      // forward it immediately. this should be as fast as
      // possible because this runs on the processor thread

      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      if ( event.type != Steinberg::Vst::Event::kNoteOnEvent ) return;

      // push to a non-blocking queue
      m_queue.push( {
        .channel = event.noteOn.channel,
        .pitch = event.noteOn.pitch,
        .velocity = event.noteOn.velocity
      } );
    }

    void initialize( sf::RenderWindow & window )
    {
      LOG_INFO( "initializing event receiver" );
      if ( !ImGui::SFML::Init( window ) )
        LOG_ERROR( "initializing imgui failed" );

      m_windowInfo.windowSize = window.getSize();
      m_windowInfo.windowView.setSize( { static_cast< float >(m_windowInfo.windowSize.x), ( float )m_windowInfo.windowSize.y } );
    }

    void shutdown( const sf::RenderWindow & window )
    {
      LOG_INFO( "shutting down event receiver" );
      ImGui::SFML::Shutdown( window );
      m_midiGen.stop();
    }

    // gets called whenever the OS indicates it's time to update
    bool executeFrame( sf::RenderWindow & window )
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
            m_windowInfo.hideMenu = !m_windowInfo.hideMenu;
        }
      }

      // update
      {
        const auto delta = m_timer.restart();
        ImGui::SFML::Update( window, delta );
        consumeMidiEvents();
        m_channelPipeline.update( delta );
      }

      // draw
      {
        window.clear();

        // the problem seems to be part of this!
        m_channelPipeline.draw( window );

        if ( !m_windowInfo.hideMenu )
          drawMenu();

        ImGui::SFML::Render( window );

        // window.setView( m_windowInfo.windowView );
        window.display();
      }

      return true;
    }

    void onResize( sf::RenderWindow & window, uint32_t width, uint32_t height )
    {
      m_windowInfo.windowSize = { width, height };
      m_windowInfo.windowView.setSize( { static_cast< float >(width),
                                              static_cast< float >(height) } );
    }

  private:

    void drawMenu()
    {
      // draw ImGui last so it sits on top of the screen
      ImGui::Begin( "nxvfx", nullptr, ImGuiWindowFlags_AlwaysAutoResize );
      ImGui::Text( "FPS %0.2f", ImGui::GetIO().Framerate );
      // TODO: draw ONLY the active one
      m_channelPipeline.drawMenu();

      if ( m_midiGen.isRunning() )
      {
        if ( ImGui::Button( "Stop MIDI" ) )
          m_midiGen.stop();
      }
      else
      {
        if ( ImGui::Button( "Start MIDI" ) )
          m_midiGen.run( m_onEvent );
      }

      ImGui::Separator();
      ImGui::Text( "%s", BUILD_NUMBER );
      ImGui::End();
    }

    void consumeMidiEvents()
    {
      // this occurs on the controller thread
      Midi_t midiEvent;
      while ( m_queue.try_pop( midiEvent ) )
        m_channelPipeline.processMidiEvent( midiEvent );
    }

  private:

    test::MidiGenerator m_midiGen { 1, 1000 };

    GlobalInfo_t m_windowInfo;

    sf::Clock m_timer;

    // only one right now
    ChannelPipeline m_channelPipeline { m_windowInfo };

    // receives midi events on the processor thread
    // and processes them on the controller thread
    Concurrency::concurrent_queue< Midi_t > m_queue;

    const std::function< void( const Steinberg::Vst::Event& ) > m_onEvent
    {
      [ & ]( const Steinberg::Vst::Event & event )
      {
        processVstEvent( event );
      }
    };

  };

}