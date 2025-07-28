/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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

  void EventFacadeApp::processAudioData(AudioDataBuffer & buffer)
  {
    m_fftBuffer.write( buffer );
  }

  void EventFacadeApp::initialize( sf::RenderWindow & window )
  {
    LOG_INFO( "initializing event receiver" );
    if ( !ImGui::SFML::Init( window ) )
      LOG_ERROR( "initializing imgui failed" );

    onResize( window, window.getSize().x, window.getSize().y );
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

    // don't anger the OS gods by spamming it too much
    if ( !window.hasFocus() && m_focusTimer.getElapsedTime().asSeconds() > 2.0f )
    {
      window.requestFocus();
      m_focusTimer.restart();
    }

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
      m_pipelines.processAudioData( m_fftBuffer );
      m_pipelines.update( delta );
    }

    // draw
    {
      window.clear();

      // the problem seems to be part of this!
      m_pipelines.draw( window );

      if ( !m_globalInfo.hideMenu )
      {
        drawMenu();
        drawDebugOverlay( window );
        // m_serialGenerator.drawMenu();
      }

      ImGui::SFML::Render( window );

      window.setView( m_ctx.globalInfo.windowView );
      window.display();
    }

    return true;
  }

  void EventFacadeApp::onResize( sf::RenderWindow & window, uint32_t width, uint32_t height )
  {
    m_globalInfo.windowSize = { width, height };
    m_globalInfo.windowView.setSize( { static_cast< float >(width),
                                            static_cast< float >(height) } );

    m_globalInfo.windowView.setCenter( m_globalInfo.windowView.getSize() / 2.f );

    m_globalInfo.windowHalfSize =
    {
      static_cast< float >( width )  / 2,
      static_cast< float >( height ) / 2
    };
  }

  void EventFacadeApp::drawMenu()
  {
    drawGenerators();
    m_pipelines.drawMenu();
  }

  void EventFacadeApp::drawGenerators()
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
      midiGen.drawMenu();

    ImGui::SeparatorText( "Audio Generator" );

    if ( m_audioGenIsRunning )
    {
      if ( ImGui::Button( "Stop Audio" ) )
      {
        m_audioGeneratorMixer.stop();
        m_audioGenIsRunning = false;
      }
    }
    else
    {
      if ( ImGui::Button( "Start Audio" ) )
      {
        m_audioGenIsRunning = true;
        // m_audioGeneratorMixer.reset();
        m_audioGeneratorMixer.run();
      }
    }

    m_audioGeneratorMixer.drawMenu();

    ImGui::End();
  }

  void EventFacadeApp::consumeMidiEvents()
  {
    // this occurs on the controller thread
    Midi_t midiEvent;
    while ( m_queue.try_dequeue( midiEvent ) )
      m_pipelines.processMidiEvent( midiEvent );
  }

  void EventFacadeApp::drawDebugOverlay( const sf::RenderWindow & window )
  {
    const auto& io = ImGui::GetIO();

    ImGui::SetNextWindowBgAlpha(0.25f); // semi-transparent
    ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Always);
    ImGui::Begin("Input Debug Overlay", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::Text("Window Focus:    %s", window.hasFocus()      ? "Yes" : "No");
    ImGui::Text("ImGui  Mouse:    %s", io.WantCaptureMouse    ? "Yes" : "No");
    ImGui::Text("ImGui  Keyboard: %s", io.WantCaptureKeyboard ? "Yes" : "No");

    if (!window.hasFocus())
    {
      ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Window lost focus!");
    }

    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard && window.hasFocus())
    {
      ImGui::TextColored(ImVec4(1, 0.7f, 0.2f, 1), "Input not captured!");
    }

    ImGui::End();
  }

}