#include "vst/EventFacadeVst.hpp"

namespace nx
{

    EventFacadeVst::EventFacadeVst( VSTStateContext& stateContext )
      : m_pipelineContext( m_globalInfo, stateContext ),
        m_pipelines( m_pipelineContext )
    {}

    void EventFacadeVst::onKeyDown( const Steinberg::char16 key,
                                    const Steinberg::int16 keyCode,
                                    const Steinberg::int16 modifiers)
    {
      ImGuiIO& io = ImGui::GetIO();

      // Add printable characters only
      if (key >= 32 && key != 127)  // skip control characters like DEL
        io.AddInputCharacter(key);

      // Map modifiers
      io.KeyCtrl  = modifiers & Steinberg::KeyModifier::kControlKey;
      io.KeyShift = modifiers & Steinberg::KeyModifier::kShiftKey;
      io.KeyAlt   = modifiers & Steinberg::KeyModifier::kAlternateKey;
      io.KeySuper = modifiers & Steinberg::KeyModifier::kCommandKey;

      // Map special keys
      switch (keyCode)
      {
        case Steinberg::VirtualKeyCodes::KEY_BACK:
          io.AddKeyEvent( ImGuiKey_Backspace, true );
          break;
        case Steinberg::VirtualKeyCodes::KEY_DELETE:
          io.AddKeyEvent( ImGuiKey_Delete, true );
          break;
        case Steinberg::VirtualKeyCodes::KEY_LEFT:
          io.AddKeyEvent( ImGuiKey_LeftArrow, true );
          break;
        case Steinberg::VirtualKeyCodes::KEY_RIGHT:
          io.AddKeyEvent( ImGuiKey_RightArrow, true );
          break;
        default:
          break;
      }
    }


    void EventFacadeVst::onKeyUp( Steinberg::char16 key, Steinberg::int16 keyCode, Steinberg::int16 modifiers )
    {
      ImGuiIO& io = ImGui::GetIO();

      // Map modifiers
      io.KeyCtrl  = modifiers & Steinberg::KeyModifier::kControlKey;
      io.KeyShift = modifiers & Steinberg::KeyModifier::kShiftKey;
      io.KeyAlt   = modifiers & Steinberg::KeyModifier::kAlternateKey;
      io.KeySuper = modifiers & Steinberg::KeyModifier::kCommandKey;

      // Map special keys
      switch (keyCode)
      {
        case Steinberg::VirtualKeyCodes::KEY_BACK:
          io.AddKeyEvent( ImGuiKey_Backspace, false );
          break;
        case Steinberg::VirtualKeyCodes::KEY_DELETE:
          io.AddKeyEvent( ImGuiKey_Delete, false );
          break;
        case Steinberg::VirtualKeyCodes::KEY_LEFT:
          io.AddKeyEvent( ImGuiKey_LeftArrow, false );
          break;
        case Steinberg::VirtualKeyCodes::KEY_RIGHT:
          io.AddKeyEvent( ImGuiKey_RightArrow, false );
          break;
        default:
          break;
      }
    }

    void EventFacadeVst::saveState( nlohmann::json &j ) const
    {
      j = m_pipelines.saveState();
    }

    void EventFacadeVst::restoreState( nlohmann::json &j )
    {
      m_pipelines.restoreState( j );
    }

    void EventFacadeVst::processVstEvent( const Steinberg::Vst::Event & event )
    {
      // forward it immediately. this should be as fast as
      // possible because this runs on the processor thread

      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      if ( event.type != Steinberg::Vst::Event::kNoteOnEvent ) return;

      // push to a non-blocking queue
      // until the next frame gets executed by the controller thread
      m_queue.enqueue( {
        .channel = event.noteOn.channel,
        .pitch = event.noteOn.pitch,
        .velocity = event.noteOn.velocity
      } );
    }

    void EventFacadeVst::processBPMChange( const double bpm )
    {
      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      m_globalInfo.bpm = bpm;
    }

    void EventFacadeVst::processPlayheadUpdate( const double playhead )
    {
      // WARNING:
      // comes in on the processor thread and not on the controller thread!
      m_globalInfo.playhead = playhead;
    }

    void EventFacadeVst::initialize( sf::RenderWindow & window )
    {
      LOG_INFO( "initializing event receiver" );
      if ( !ImGui::SFML::Init( window ) )
      {
        LOG_ERROR( "failed to initialize imgui" );
      }

      onResize( window, window.getSize().x, window.getSize().y );
    }

    void EventFacadeVst::shutdown( sf::RenderWindow & window )
    {
      LOG_INFO( "shutting down event receiver" );
      ImGui::SFML::Shutdown( window );
      m_pipelines.shutdown();
    }

    bool EventFacadeVst::executeFrame( sf::RenderWindow & window )
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

        // synchronize the delta amounts even though it's not the absolute runtime, it's close enough
        m_globalInfo.elapsedTimeSeconds += delta.asSeconds();
        ++m_globalInfo.frameCount;

        ImGui::SFML::Update( window, delta );
        consumeMidiEvents();
        m_pipelines.update( delta );
      }

      // draw
      {
        window.clear();

        m_pipelines.draw( window );
        drawMenu();
        ImGui::SFML::Render( window );

        window.setView( m_globalInfo.windowView );
        window.display();
      }

      return true;
    }

    void EventFacadeVst::onResize( sf::RenderWindow & window, uint32_t width, uint32_t height )
    {
      m_globalInfo.windowSize = { width, height };
      m_globalInfo.windowHalfSize = { static_cast< float >(width) / 2.f, static_cast< float >(height) / 2.f };
      window.setSize( m_globalInfo.windowSize );
      m_globalInfo.windowView.setSize( { static_cast< float >(width), static_cast< float >(height) } );
      m_globalInfo.windowView.setCenter( m_globalInfo.windowHalfSize );
    }

    void EventFacadeVst::drawMenu()
    {
      if ( m_pipelineContext.globalInfo.hideMenu ) return;

      m_pipelines.drawMenu();
    }

    void EventFacadeVst::consumeMidiEvents()
    {
      // this occurs on the controller thread
      Midi_t midiEvent;
      while ( m_queue.try_dequeue( midiEvent ) )
        m_pipelines.processMidiEvent( midiEvent );
    }

}