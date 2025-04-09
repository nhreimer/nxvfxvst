#pragma once

#include <unordered_map>
#include <concurrent_queue.h>
#include <fmt/core.h>

#include <SFML/Graphics/RenderWindow.hpp>

#include "pluginterfaces/vst/ivstevents.h"

#include "models/data/GlobalInfo_t.hpp"
#include "models/ChannelPipeline.hpp"
#include "models/MultichannelPipeline.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

#include "log/Logger.hpp"

namespace nx
{

  // Event Facade for the VST3 plugin
  class EventFacadeVst final
  {
  public:

    EventFacadeVst()
      : m_pipelines( m_globalInfo )
    {}

    ~EventFacadeVst() = default;

    void saveState( nlohmann::json &j ) const
    {
      j = m_pipelines.saveState();
    }

    void restoreState( nlohmann::json &j )
    {
      m_pipelines.restoreState( j );
    }

    void processVstEvent( const Steinberg::Vst::Event & event )
    {
      // forward it immediately. this should be as fast as
      // possible because this runs on the processor thread

      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      if ( event.type != Steinberg::Vst::Event::kNoteOnEvent ) return;

      // push to a non-blocking queue
      // until the next frame gets executed by the controller thread
      m_queue.push( {
        .channel = event.noteOn.channel,
        .pitch = event.noteOn.pitch,
        .velocity = event.noteOn.velocity
      } );
    }

    void processBPMChange( const double bpm )
    {
      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      m_globalInfo.bpm = bpm;
    }

    void initialize( sf::RenderWindow & window )
    {
      LOG_INFO( "initializing event receiver" );
      if ( !ImGui::SFML::Init( window ) )
      {
        LOG_ERROR( "failed to initialize imgui" );
      }

      onResize( window, window.getSize().x, window.getSize().y );
    }

    void shutdown( sf::RenderWindow & window )
    {
      LOG_INFO( "shutting down event receiver" );
      ImGui::SFML::Shutdown( window );
    }

    bool executeFrame( sf::RenderWindow & window )
    {
      if ( !window.isOpen() ) return false;

      while ( const std::optional event = window.pollEvent() )
      {
        // post events
        ImGui::SFML::ProcessEvent( window, *event );

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

        // TODO: gotta forward this differently
        //m_channelPipeline.processEvent( event );
      }

      // update
      {
        const auto delta = m_clock.restart();
        ImGui::SFML::Update( window, delta );
        consumeMidiEvents();
        m_pipelines.update( delta );
      }

      // draw
      {
        window.clear();

        // the problem seems to be part of this!
        m_pipelines.draw( window );
        drawMenu();
        ImGui::SFML::Render( window );

        window.setView( m_globalInfo.windowView );
        window.display();
      }

      return true;
    }

    void onResize( sf::RenderWindow & window, uint32_t width, uint32_t height )
    {
      m_globalInfo.windowSize = { width, height };
      window.setSize( m_globalInfo.windowSize );
      m_globalInfo.windowView.setSize( { static_cast< float >(width), static_cast< float >(height) } );
      m_globalInfo.windowView.setCenter( { static_cast< float >(width) / 2.f, static_cast< float >(height) / 2.f } );
    }

  private:

    void drawMenu()
    {
      if ( m_globalInfo.hideMenu ) return;

      m_pipelines.drawMenu();
    }

    void consumeMidiEvents()
    {
      // this occurs on the controller thread
      Midi_t midiEvent;
      while ( m_queue.try_pop( midiEvent ) )
        m_pipelines.processMidiEvent( midiEvent );
    }

  private:

    GlobalInfo_t m_globalInfo;
    MultichannelPipeline m_pipelines;

    sf::Clock m_clock;

    // receives midi events on the processor thread
    // and processes them on the controller thread
    Concurrency::concurrent_queue< Midi_t > m_queue;
  };

}