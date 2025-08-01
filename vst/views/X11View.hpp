#pragma once

#include <chrono>
#include <thread>

#include <X11/Xlib.h>

#include <base/source/fobject.h>
#include <pluginterfaces/gui/iplugview.h>
#include <pluginterfaces/base/funknown.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/ContextSettings.hpp>

#include "log/Logger.hpp"
#include "vst/EventFacadeVst.hpp"
#include "vst/VSTStateContext.hpp"
#include "vst/views/IVSTView.hpp"

namespace nx
{
  class X11View final : public Steinberg::FObject,
                          public IVSTView,    // same as IPlugView but with notify() added
                          public Steinberg::IPlugFrame
  {

  private:
    struct X11Details_t
    {
      Display * display { nullptr };
      ::Window parentHwnd { 0 };
      ::Window childHwnd { 0 };
      int32_t screen { -1 };
      bool isRunning { false };
      std::unique_ptr< std::thread > processingThread;
      std::unique_ptr< sf::RenderWindow > renderThread;
    };

  public:

    explicit X11View( VSTStateContext& stateContext,
                        const Steinberg::ViewRect windowSize,
                        std::function< void( IVSTView * ) >&& onRemoved )
      : m_stateContext( stateContext ),
        m_rect( windowSize ),
        m_onRemoved( onRemoved ),
        m_eventFacade( stateContext )
    {}

    ~X11View() override
    {
      if ( m_details.isRunning )
        removed();
    }

    void saveState(nlohmann::json& j) override { m_eventFacade.saveState( j ); }
    void restoreState(nlohmann::json& j) override { m_eventFacade.restoreState( j ); }

    ////////////////////////////////////////////////////////////////////////////////
    Steinberg::tresult isPlatformTypeSupported( Steinberg::FIDString type ) override
    {
      // Windows platform
      if ( strcmp( type, Steinberg::kPlatformTypeX11EmbedWindowID ) == 0 )
        return Steinberg::kResultTrue;

      return Steinberg::kResultFalse;
    }

    ////////////////////////////////////////////////////////////////////////////////
    /***
     * This loads our own Child Window that we will control and display.
     * @param parent parent window handle
     * @param type
     * @return kResultOk/kResultTrue if successful, otherwise false
     */
    Steinberg::tresult attached( void * parent, Steinberg::FIDString type ) override
    {
      if ( m_details.isRunning )
      {
        LOG_ERROR( "X11View already attached" );
        return Steinberg::kResultFalse;
      }

      if ( !createChildWindow( parent ) )
        return Steinberg::kResultFalse;

      initializeRenderWindow();
      startMessagePump();

      return Steinberg::kResultTrue;
    }

    ////////////////////////////////////////////////////////////////////////////////
    Steinberg::tresult removed() override
    {
      m_isActive = false;
      LOG_DEBUG( "event processing turned off" );
      m_onRemoved( this );
      m_details.isRunning = false;

      LOG_DEBUG( "shutting down processing loop" );
      if ( m_details.processingThread->joinable() )
        m_details.processingThread->join();

      LOG_DEBUG( "shutting down X11 display" );
      if ( m_details.display && m_details.childHwnd )
      {
        ::XDestroyWindow( m_details.display, m_details.childHwnd );
        ::XCloseDisplay( m_details.display );

        m_details.display = nullptr;
        m_details.childHwnd = 0;
      }

      return Steinberg::kResultTrue;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void notify( Steinberg::Vst::IMessage * rawMessage ) override
    {
      // NOTE: don't do anything CPU intensive here
      m_eventFacade.processVstEvent( rawMessage );
    }

    ////////////////////////////////////////////////////////////////////////////////
    void notify( Steinberg::Vst::Event& event ) override
    {
      // NOTE: don't do anything CPU intensive here
      m_eventFacade.processVstEvent( event );
    }

    ////////////////////////////////////////////////////////////////////////////////
    void notifyBPMChange( const double bpm ) override
    {
      // NOTE: don't do anything CPU intensive here
      m_eventFacade.processBPMChange( bpm );
    }

    ////////////////////////////////////////////////////////////////////////////////
    void notifyPlayheadUpdate( const double playhead ) override
    {
      m_eventFacade.processPlayheadUpdate( playhead );
    }

    ////////////////////////////////////////////////////////////////////////////////
    void notifySampleRate( const double sampleRate ) override
    {
      m_eventFacade.processSampleRateUpdate( sampleRate );
    }


  ////////////////////////////////////////////////////////////////////////////////
  Steinberg::tresult onWheel( float distance ) override
  {
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult onKeyDown( Steinberg::char16 key, Steinberg::int16 keyCode, Steinberg::int16 modifiers ) override
  {
    m_eventFacade.onKeyDown( key, keyCode, modifiers );
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult onKeyUp( Steinberg::char16 key, Steinberg::int16 keyCode, Steinberg::int16 modifiers ) override
  {
    m_eventFacade.onKeyUp( key, keyCode, modifiers );
    return Steinberg::kResultTrue;
  }

  ////////////////////////////////////////////////////////////////////////////////
  Steinberg::tresult getSize( Steinberg::ViewRect *size ) override
  {
    *size = m_rect;
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult onSize( Steinberg::ViewRect *newSize ) override
  {
    // this is where the actual resize occurs NOT in resizeView
    m_rect = *newSize;
    m_eventFacade.onResize( *m_details.renderThread.get(), newSize->getWidth(), newSize->getHeight() );
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult canResize() override
  {
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult checkSizeConstraint( Steinberg::ViewRect *rect ) override
  {
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult resizeView( IPlugView *view, Steinberg::ViewRect *newSize ) override
  {
    m_eventFacade.onResize( *m_details.renderThread.get(), newSize->getWidth(), newSize->getHeight() );
    return Steinberg::kResultTrue;
  }

  ////////////////////////////////////////////////////////////////////////////////
  Steinberg::tresult onFocus( Steinberg::TBool state ) override
  {
    return Steinberg::kResultTrue;
  }

  Steinberg::tresult setFrame( Steinberg::IPlugFrame *frame ) override
  {
    LOG_DEBUG( "setFrame" );
    return Steinberg::kResultFalse;
  }

  ////////////////////////////////////////////////////////////////////////////////

  void setFrameRate( const int32_t fps ) override
  {
    LOG_WARN( "unable to set frame rate for X11" );
  }

  ////////////////////////////////////////////////////////////////////////////////

  int32_t getFrameRate() const override { return FPS; }

  ////////////////////////////////////////////////////////////////////////////////

  //---Interface------
  OBJ_METHODS (X11View, FObject)
  DEFINE_INTERFACES
    DEF_INTERFACE (IPlugView)
    DEF_INTERFACE (IPlugFrame)
  END_DEFINE_INTERFACES (FObject)
  REFCOUNT_METHODS (FObject)


  private:

    ////////////////////////////////////////////////////////////////////////////////
    bool createChildWindow( void * parent )
    {
      m_details.display = ::XOpenDisplay( nullptr );

      if ( m_details.display == nullptr )
      {
        LOG_ERROR( "Failed to open X11 display" );
        return false;
      }

      m_details.parentHwnd = reinterpret_cast< ::Window >( parent );
      m_details.screen = DefaultScreen( m_details.display );

      // XCreateSimpleWindow inherits its attributes from its parent
      m_details.childHwnd = ::XCreateSimpleWindow(
        m_details.display,
        m_details.parentHwnd,
        0,
        0,
        m_rect.getWidth(),
        m_rect.getHeight(),
        1,
        BlackPixel( m_details.display, m_details.screen ),
        WhitePixel( m_details.display, m_details.screen ) );

      ::XMapWindow( m_details.display, m_details.childHwnd );
      ::XFlush( m_details.display );

      return true;
    }

    /////////////////////////////////////////////////////////////////////////////
    void initializeRenderWindow()
    {
      LOG_DEBUG( "initializing SFML render window" );
      m_details.renderThread->create( m_details.childHwnd, m_sfContext );
      m_details.renderThread->setVerticalSyncEnabled( false );
      m_details.renderThread->setFramerateLimit( 0 );

      LOG_INFO( "SFML Render Window OpenGL version {}.{}.",
        m_sfContext.majorVersion,
        m_sfContext.minorVersion );

      if ( !m_details.renderThread->setActive( true ) )
      {
        LOG_WARN( "failed to activate window." );
      }

      m_details.renderThread->requestFocus();
      m_details.renderThread->clear();
      m_details.renderThread->display();
    }

    /////////////////////////////////////////////////////////////////////////////
    void startMessagePump()
    {
      m_details.processingThread = std::make_unique< std::thread >(
        [this]()
        {
          m_details.renderThread = std::make_unique< sf::RenderWindow >();
          initializeRenderWindow();

          // initialize prior to the message pump for ImGui
          m_eventFacade.initialize( *m_details.renderThread.get() );

          m_details.isRunning = true;

          while ( m_details.isRunning )
            m_eventFacade.executeFrame( *m_details.renderThread.get() );

          m_eventFacade.shutdown( *m_details.renderThread.get() );

          if ( m_details.renderThread->isOpen() )
            m_details.renderThread->close();
        }
      );
    }

  private:

    VSTStateContext& m_stateContext;

    // used for the initial size only
    Steinberg::ViewRect m_rect;

    // used for notifying the controller that we're closing and
    // state or cleanup is required
    std::function< void( IVSTView* ) > m_onRemoved;

    bool m_isActive { false };

    EventFacadeVst m_eventFacade;

    // unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0, unsigned int major = 1, unsigned int minor = 1, unsigned int attributes = Default, bool sRgb = false
    sf::ContextSettings m_sfContext { 24, 0, 8, 4, 6 };

    X11Details_t m_details;

    // our frame rate is fixed in linux for the time being
    inline static int32_t FRAME_RATE_MS = 16;
    inline static int32_t FPS = 60; // approximately
  };
}