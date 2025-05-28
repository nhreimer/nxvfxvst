#pragma once

#include <Windows.h>
#include <rpc.h>

#include <base/source/fobject.h>
#include <pluginterfaces/gui/iplugview.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/ivstmessage.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/ContextSettings.hpp>

#include "log/Logger.hpp"
#include "vst/EventFacadeVst.hpp"
#include "vst/VSTStateContext.hpp"
#include "vst/views/IVSTView.hpp"

///////////////////////////////////////////////////////////
/// DEPRECATED IN FAVOR OF USING Win32's TimerQueue API,
/// which is more consistent when compared to SetTimer API
/// This change was made to help lock in video recording
/// better and not for any other reason.
///////////////////////////////////////////////////////////

namespace nx
{

namespace priv
{
  ////////////////////////////////////////////////////////////////////////////////
  /// WIN32 WINDOW INFO
  ////////////////////////////////////////////////////////////////////////////////
  struct Win32Details_t
  {
    std::string classname;              // WNDCLASS classname. must be unique.
    HWND parentHwnd { nullptr };        // Parent HWND of the child HWND
    HWND childHwnd { nullptr };         // HWND to the child
    // HINSTANCE hInstance { nullptr };    // Instance handle: not necessary
    UINT_PTR timerResult { 0 };         // timer id
  };

  // NOTE: this should probably be thread safe to be on the safe side
  class WinImpl
  {
  public:

    static HWND createChildWindow( const HWND parent, const sf::IntRect r )
    {
      if ( parent == nullptr )
      {
        LOG_CRITICAL( "Parent HWND must not be null!" );
        return nullptr;
      }

      const auto moduleHandle = getWindowsModuleHandle();

      // LOG_DEBUG( "ModuleHandle: {}", static_cast< void * >( moduleHandle ) );

      // failed to register? we only need to register a single class for n number of parents
      if ( !registerWin32Class( moduleHandle ) ) return nullptr;

      // successfully registered
      m_isRegistered = true;

      HWND handle = ::CreateWindowEx( WS_EX_CONTROLPARENT,		 // Extended possibilities for variation
                                      INTERNAL_WINDOW_NAME,    // window class name
                                      WINDOW_NAME,	           // Title Text
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS, // flags
                                      r.position.x, r.position.y, r.size.x, r.size.y, // pos & size
                                      //r.left, r.top, r.width, r.height,                   // pos & size
                                      parent,	                                              // the parent window
                                      nullptr,                                              // menu
                                      moduleHandle,                                         // HINSTANCE of calling application
                                      nullptr );                                     // idk

      if ( handle == nullptr )
      {
        LOG_ERROR( "CreateWindowEx failed: {}", ::GetLastError() );
        return nullptr;
      };

      LOG_DEBUG( "CreateWindowEx successful: {}", static_cast< void * >( handle ) );

      return handle;
    }

  private:

    static bool registerWin32Class( HINSTANCE moduleHandle )
    {
      if ( m_isRegistered )
      {
        LOG_DEBUG( "window class already registered. skipping step." );
        return true;
      }

      WNDCLASSEX wincl;
      wincl.hInstance = moduleHandle;
      wincl.lpszClassName = INTERNAL_WINDOW_NAME;
      wincl.lpfnWndProc = &processWndEvent;
      wincl.style = CS_DBLCLKS | CS_OWNDC;
      wincl.cbSize = sizeof( wincl );
      wincl.hIcon = ::LoadIcon( nullptr, IDI_APPLICATION );
      wincl.hIconSm = wincl.hIcon;
      wincl.hCursor = ::LoadCursor(nullptr, IDC_ARROW );
      wincl.lpszMenuName = nullptr;
      wincl.cbClsExtra = 0;
      wincl.cbWndExtra = 0;
      wincl.hbrBackground = reinterpret_cast< HBRUSH >( COLOR_BTNFACE + 1 );

      if ( ::RegisterClassEx( &wincl ) == 0 )
      {
        LOG_ERROR( "Failed to register window class: {}", ::GetLastError() );
        return false;
      }

      LOG_DEBUG( "Successfully registered window class" );
      return true;
    }

    static HINSTANCE getWindowsModuleHandle()
    {
      MEMORY_BASIC_INFORMATION mbi;
      static int dummy;
      ::VirtualQuery( &dummy, &mbi, sizeof( mbi ) );
      return static_cast< HINSTANCE >( mbi.AllocationBase );
    }

    static LRESULT processWndEvent( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
    {
      return ::DefWindowProc( hwnd, msg, wParam, lParam );
    }

   static inline const auto INTERNAL_WINDOW_NAME = L"nxInternalWindow";
   static inline const auto WINDOW_NAME = L"nxChildWindow";

   // we only need to register a window once
   static inline bool m_isRegistered { false };
  };

}

  ////////////////////////////////////////////////////////////////////////////////
  /// WIN32 CHILD VIEW
  ////////////////////////////////////////////////////////////////////////////////
  class Win32View final : public Steinberg::FObject,
                          public IVSTView,    // same as IPlugView but with notify() added
                          public Steinberg::IPlugFrame
{
  public:
    void saveState(nlohmann::json &j) override { m_eventFacade.saveState( j ); }
    void restoreState(nlohmann::json &j) override { m_eventFacade.restoreState( j ); }

    ////////////////////////////////////////////////////////////////////////////////
    explicit Win32View( VSTStateContext& stateContext,
                        const Steinberg::ViewRect windowSize,
                        std::function< void( IVSTView * ) >&& onRemoved )
      : m_stateContext( stateContext ),
        m_rect( windowSize ),
        m_onRemoved( onRemoved ),
        m_eventFacade( stateContext )
    {}

    ////////////////////////////////////////////////////////////////////////////////
    ~Win32View() override
    {
      // run the window shutdown process
      // notify the event receiver that the window will close
      m_eventFacade.shutdown( m_sfWindow );

      if ( m_win32.childHwnd != nullptr )
      {
        // stop the callbacks
        if ( m_win32.timerResult != 0 )
        {
          ::KillTimer( m_win32.childHwnd, m_win32.timerResult );
          m_win32.timerResult = 0;
        }

        if ( const auto it = sm_wndMap.find( m_win32.childHwnd ); it != sm_wndMap.end() )
          sm_wndMap.erase( it );
        else // something has gone wrong!
          LOG_ERROR( "unable to properly shut down child HWND because it is not mapped!" );

        // shutdown can get called prior to the destructor, so mark this as invalid
        m_win32.childHwnd = nullptr;

        LOG_INFO( "child window has been closed" );
      }
      else
        LOG_WARN( "child window has already been shut down" );
    }

  ////////////////////////////////////////////////////////////////////////////////
  void notify( Steinberg::Vst::IMessage * rawMsg ) override
  {
    // NOTE: don't do anything CPU intensive here!!!
    m_eventFacade.processVstEvent( rawMsg );
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

  void setFrameRate( const int32_t fps ) override
  {
    // ignored
  }

  ////////////////////////////////////////////////////////////////////////////////

  int32_t getFrameRate() const override { return USER_TIMER_MINIMUM / 60; }

  ////////////////////////////////////////////////////////////////////////////////

  //---Interface------
  OBJ_METHODS (Win32View, FObject)
  DEFINE_INTERFACES
    DEF_INTERFACE (IPlugView)
    DEF_INTERFACE (IPlugFrame)
  END_DEFINE_INTERFACES (FObject)
  REFCOUNT_METHODS (FObject)

private:

  /////////////////////////////////////////////////////////////////////////////
  static void WINAPI processTimerExpiry( HWND hwnd,
                                         UINT wmTimerMsg,
                                         UINT_PTR timerId,
                                         DWORD currentSysTime )
  {
    if ( const auto it = sm_wndMap.find( hwnd ); it != sm_wndMap.end() )
    {
      if ( !it->second->m_eventFacade.executeFrame( it->second->m_sfWindow ) )
      {
        LOG_DEBUG( "event loop indicates window is shutting down" );
      }
    }
    else
    {
      // this means something has gone terribly wrong. events cannot be processed,
      // so kill the timer.
      LOG_CRITICAL( "timer is running but child HWND not found. stopping timer." );
      ::KillTimer( hwnd, timerId );
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  void initializeRenderWindow()
  {
    LOG_DEBUG( "initializing SFML render window" );
    m_sfWindow.create( m_win32.childHwnd, m_sfContext );

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
    m_win32.timerResult = ::SetTimer( m_win32.childHwnd,
                                      sm_timerIdCounter,
                                      USER_TIMER_MINIMUM,
                                      processTimerExpiry );

    if ( m_win32.timerResult == 0 )
    {
      LOG_ERROR( "failed to create windows timer: {}", ::GetLastError() );
      return false;
    }

    ++sm_timerIdCounter;
    return true;
  }

  /////////////////////////////////////////////////////////////////////////////
  bool createChildWindow( HWND parentHwnd )
  {

    LOG_INFO( "creating child window" );

    const auto childHwnd = priv::WinImpl::createChildWindow(
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

    sm_wndMap.insert( { m_win32.childHwnd, this } );
    LOG_INFO( "inserted child window to static map" );

    initializeRenderWindow();

    if ( !startMessagePump() )
    {
      LOG_ERROR( "failed to start message pump." );
      return false;
    }

    return true;
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

private:

  VSTStateContext& m_stateContext;

  // used for the initial size only
  Steinberg::ViewRect m_rect;

  // used for notifying the controller that we're closing and
  // state or cleanup is required
  std::function< void( IVSTView* ) > m_onRemoved;

  bool m_isActive { false };

  nx::EventFacadeVst m_eventFacade;

  // SFML specifics
  sf::RenderWindow m_sfWindow;

  // unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0, unsigned int major = 1, unsigned int minor = 1, unsigned int attributes = Default, bool sRgb = false
  sf::ContextSettings m_sfContext { 24, 0, 8, 4, 6 };

  // holds Win32 window specifics
  priv::Win32Details_t m_win32;

  // required for timer callbacks across multiple instances
  // each child HWND is mapped to Win32SfmlWindow instance
  inline static std::unordered_map< HWND, Win32View * > sm_wndMap {};

  // usually starts at 5 and goes up
  inline static std::atomic< uint32_t > sm_timerIdCounter { 5 };

};

}