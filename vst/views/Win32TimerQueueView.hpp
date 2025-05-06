#pragma once

#include <Windows.h>
#include <dwmapi.h>
#include <rpc.h>

#include <base/source/fobject.h>
#include <pluginterfaces/gui/iplugview.h>
#include <pluginterfaces/base/funknown.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/ContextSettings.hpp>

#include "log/Logger.hpp"
#include "vst/EventFacadeVst.hpp"
#include "vst/VSTStateContext.hpp"
#include "vst/views/IVSTView.hpp"
#include "vst/views/Win32WinImpl.hpp"


///////////////////////////////////////////////////////////
/// Uses the Win32 TimerQueue API rather than SetTimer
/// for more consistent frame rates. Although we can set the
/// timer to 1 ms if we wanted, we won't get any real gains
/// because we can't bypass DWM reliably or safely.
///
/// To prevent the timer from spamming us unnecessarily,
/// the class will match the refresh rate reported by Windows.
///////////////////////////////////////////////////////////

namespace nx
{
  ////////////////////////////////////////////////////////////////////////////////
  /// WIN32 CHILD VIEW
  ////////////////////////////////////////////////////////////////////////////////
  class Win32TimerQueueView final : public Steinberg::FObject,
                          public IVSTView,    // same as IPlugView but with notify() added
                          public Steinberg::IPlugFrame
{
  public:
    void saveState(nlohmann::json &j) override { m_eventFacade.saveState( j ); }
    void restoreState(nlohmann::json &j) override { m_eventFacade.restoreState( j ); }

    ////////////////////////////////////////////////////////////////////////////////
    explicit Win32TimerQueueView( VSTStateContext& stateContext,
                        const Steinberg::ViewRect windowSize,
                        std::function< void( IVSTView * ) >&& onRemoved )
      : m_stateContext( stateContext ),
        m_rect( windowSize ),
        m_onRemoved( onRemoved ),
        m_eventFacade( stateContext )
    {}

    ////////////////////////////////////////////////////////////////////////////////
    ~Win32TimerQueueView() override
    {
      // run the window shutdown process
      // notify the event receiver that the window will close
      m_eventFacade.shutdown( m_sfWindow );

      if ( m_win32.childHwnd != nullptr )
      {
        // stop the callbacks
        stopMessagePump();

        // shutdown can get called prior to the destructor, so mark this as invalid
        m_win32.childHwnd = nullptr;

        LOG_INFO( "child window has been closed" );
      }
      else
        LOG_WARN( "child window has already been shut down" );
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
  Steinberg::tresult isPlatformTypeSupported( Steinberg::FIDString type ) override
  {
    // Windows platform
    if ( strcmp( type, Steinberg::kPlatformTypeHWND ) == 0 )
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
    LOG_INFO( "creating child window" );
    if ( createChildWindow( static_cast< HWND >( parent ) ) )
    {
      LOG_INFO( "child window created" );
      m_isActive = true;
      LOG_DEBUG( "event processing turned on" );

      ::QueryPerformanceFrequency(&m_perfFreq);
      ::QueryPerformanceCounter(&m_lastTick);

      return Steinberg::kResultTrue;
    }

    LOG_ERROR( "unable to attach child window" );
    return Steinberg::kResultFalse;
  }

  ////////////////////////////////////////////////////////////////////////////////
  Steinberg::tresult removed() override
  {
    m_isActive = false;
    LOG_DEBUG( "event processing turned off" );
    m_onRemoved( this );
    // the destructor handles the shutdown process.
    // do NOT add another shutdown here or undefined behavior will occur
    return Steinberg::kResultTrue;
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
    m_eventFacade.onResize( m_sfWindow, newSize->getWidth(), newSize->getHeight() );
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
    m_eventFacade.onResize( m_sfWindow, newSize->getWidth(), newSize->getHeight() );
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
    m_win32.currentFPS = std::clamp( fps, 1, 240 );
    stopMessagePump();
    startMessagePump();
  }

  ////////////////////////////////////////////////////////////////////////////////

  int32_t getFrameRate() const override { return m_win32.currentFPS; }

  ////////////////////////////////////////////////////////////////////////////////

  //---Interface------
  OBJ_METHODS (Win32TimerQueueView, FObject)
  DEFINE_INTERFACES
    DEF_INTERFACE (IPlugView)
    DEF_INTERFACE (IPlugFrame)
  END_DEFINE_INTERFACES (FObject)
  REFCOUNT_METHODS (FObject)

private:

  /////////////////////////////////////////////////////////////////////////////
  void initializeRenderWindow()
  {
    LOG_DEBUG( "initializing SFML render window" );
    m_sfWindow.create( m_win32.childHwnd, m_sfContext );
    m_sfWindow.setVerticalSyncEnabled( false );
    m_sfWindow.setFramerateLimit( 0 );

    LOG_INFO( "SFML Render Window OpenGL version {}.{}.",
      m_sfContext.majorVersion,
      m_sfContext.minorVersion );
      //m_sfContext.antialiasingLevel );

    if ( !::AllowSetForegroundWindow( ASFW_ANY ) )
    {
      LOG_WARN( "failed to allow foreground. interactivity may be limited." );
    }

    if ( !m_sfWindow.setActive( true ) )
    {
      LOG_WARN( "failed to activate window." );
    }

    m_sfWindow.requestFocus();
    m_sfWindow.clear();
    m_sfWindow.display();

    // initialize prior to the message pump for ImGui
    m_eventFacade.initialize( m_sfWindow );
  }

  /////////////////////////////////////////////////////////////////////////////
  bool startMessagePump()
  {
    if ( m_win32.redrawTimer )
    {
      LOG_DEBUG( "redrawing timer already set." );
      return true;
    }

    const int intervalMs = std::max(1, 1000 / m_win32.currentFPS);

    const auto refreshRate = MathHelper::roundTo( getRefreshRateHz(), 0 );

    LOG_INFO( "refresh rate reported as {}", refreshRate );

    if ( m_win32.currentFPS > refreshRate )
    {
      LOG_INFO("FPS set too high at {}. lowered to {}.", m_win32.currentFPS, refreshRate);
      m_win32.currentFPS = refreshRate;
    }

    const BOOL success = ::CreateTimerQueueTimer(
      &m_win32.redrawTimer,
      nullptr,
      [](PVOID lpParam, BOOLEAN)
      {
        auto* self = static_cast<Win32TimerQueueView*>(lpParam);
        if (self && ::IsWindow(self->m_win32.childHwnd))
        {
          // Frame timing
          ::LARGE_INTEGER now;
          ::QueryPerformanceCounter(&now);

          const double elapsedMs =
              1000.0 * static_cast<double>(now.QuadPart - self->m_lastTick.QuadPart) / self->m_perfFreq.QuadPart;

          const double expectedMs = 1000.0 / static_cast<double>(self->m_win32.currentFPS);

          // Optional: set threshold to 1.5x expected frame time
          if (elapsedMs > expectedMs * 1.2)
          {
            // Frame dropped — log, track, or visualize
            LOG_INFO("[DropFrame] Missed frame deadline. expected {}, elapsed: {}", expectedMs, elapsedMs);
          }

          self->m_lastTick = now;

          ::PostMessage(self->m_win32.childHwnd,
                        WM_RUN_FRAME,
                        reinterpret_cast<WPARAM>( self ),
                        0);
        }
      },
      this,
      0,             // start immediately
      intervalMs,    // frame interval
      WT_EXECUTEDEFAULT
    );

    return success == TRUE;
  }

  /////////////////////////////////////////////////////////////////////////////
  void stopMessagePump()
  {
    // stop the callbacks
    if ( m_win32.redrawTimer != nullptr )
    {
      const auto result = ::DeleteTimerQueueTimer( nullptr, m_win32.redrawTimer, nullptr );
      if ( result != TRUE )
      {
        LOG_ERROR( "DeleteTimerQueueTimer failed: {}", ::GetLastError() );
      }
      m_win32.redrawTimer = nullptr;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  bool createChildWindow( HWND parentHwnd )
  {
    LOG_INFO( "creating child window" );

    const auto childHwnd = priv::Win32WinImpl::createChildWindow(
      this,
      &Win32TimerQueueView::childWindowProc,
      parentHwnd,
      { { m_rect.left, m_rect.top },
          { m_rect.getWidth(), m_rect.getHeight() } } );

    if ( childHwnd == nullptr )
    {
      LOG_CRITICAL( "failed to create child window. cannot continue." );
      return false;
    }

    LOG_INFO( "created child window" );

    m_win32.parentHwnd = parentHwnd;
    m_win32.childHwnd = childHwnd;

    initializeRenderWindow();

    if ( !startMessagePump() )
    {
      LOG_ERROR( "failed to start message pump." );
      return false;
    }

    LOG_INFO( "started message pump at {} FPS", m_win32.currentFPS );

    return true;
  }

  static double getRefreshRateHz()
  {
    ::DWM_TIMING_INFO timingInfo = {};
    timingInfo.cbSize = sizeof(timingInfo);

    LARGE_INTEGER qpcFreq = {};
    QueryPerformanceFrequency(&qpcFreq);

    if (SUCCEEDED(::DwmGetCompositionTimingInfo(nullptr, &timingInfo)))
    {
      if (timingInfo.qpcRefreshPeriod > 0 && qpcFreq.QuadPart > 0)
      {
        LOG_INFO( "Dwm reported refresh rate" );
        return static_cast<double>(qpcFreq.QuadPart) / static_cast<double>(timingInfo.qpcRefreshPeriod);
      }
      LOG_WARN( "Dwm succeeded but unable to get metrics." );
    }

    LOG_WARN( "failed to get Dwm refresh rate. attempting to get desktop rate." );
    // DWM failed — fallback to display info
    DEVMODE devMode = {};
    devMode.dmSize = sizeof(devMode);

    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
    {
      if (devMode.dmDisplayFrequency > 1)
      {
        LOG_INFO( "Windows display reported refresh rate" );
        return (devMode.dmDisplayFrequency);
      }
    }

    LOG_WARN( "failed to get desktop rate. defaulting to 60 FPS." );

    // Last resort
    return 60.f;
  }

  /***
 * Gets the window size from the OS
 * @param hwnd handle to a window
 * @return size of window if successful, otherwise a vector of { 0, 0 }
 */
  static sf::Vector2u getWin32WindowSize( const HWND hwnd )
  {
    RECT frame {};
    if ( ::GetWindowRect( hwnd, &frame ) )
    {
      return { static_cast< uint32_t >( frame.right - frame.left ),
                  static_cast< uint32_t >( frame.bottom - frame.top ) };
    }

    // hand back an empty vector
    return sf::Vector2u {};
  }

  static LRESULT CALLBACK childWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    auto* self = reinterpret_cast< Win32TimerQueueView* >( wParam );
    if ( !self )
      return ::DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
      case WM_RUN_FRAME:
        self->m_eventFacade.executeFrame(self->m_sfWindow);
        return 0;

      case WM_PAINT:
        // existing paint code, or just validate
        ::ValidateRect(hwnd, nullptr);
        return 0;

        // other cases: WM_SIZE, WM_CLOSE, etc.
      default:
        break;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
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

  // SFML specifics
  sf::RenderWindow m_sfWindow;

  // unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0, unsigned int major = 1, unsigned int minor = 1, unsigned int attributes = Default, bool sRgb = false
  sf::ContextSettings m_sfContext { 24, 0, 8, 4, 6 };

  // holds Win32 window specifics
  priv::Win32Details_t m_win32;

  static constexpr UINT WM_RUN_FRAME = WM_APP + 1;

  ::LARGE_INTEGER m_perfFreq {};
  ::LARGE_INTEGER m_lastTick {};

};

}