#pragma once

namespace nx::priv
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
    HANDLE redrawTimer { nullptr };
    int32_t currentFPS { 120 };
  };

  // NOTE: this should probably be thread safe to be on the safe side
  class Win32WinImpl
  {
  public:

    static HWND createChildWindow( void * obj,
                                   LRESULT (*childWndProc)( HWND, UINT, WPARAM, LPARAM ),
                                   const HWND parent,
                                   const sf::IntRect r )
    {

      if ( parent == nullptr )
      {
        LOG_CRITICAL( "Parent HWND must not be null!" );
        return nullptr;
      }

      const auto moduleHandle = getWindowsModuleHandle();

      // LOG_DEBUG( "ModuleHandle: {}", static_cast< void * >( moduleHandle ) );

      // failed to register? we only need to register a single class for n number of parents
      if ( !registerWin32Class( childWndProc, moduleHandle ) ) return nullptr;

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
      }

      // ::SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(obj));
      // ::SetWindowLongPtr(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(childWndProc));

      LOG_DEBUG( "CreateWindowEx successful: {}", static_cast< void * >( handle ) );

      return handle;
    }

  private:

    static bool registerWin32Class(
      LRESULT (*childWndProc)( HWND, UINT, WPARAM, LPARAM ),
      HINSTANCE moduleHandle )
    {
      if ( m_isRegistered )
      {
        LOG_DEBUG( "window class already registered. skipping step." );
        return true;
      }

      WNDCLASSEX wincl;
      wincl.hInstance = moduleHandle;
      wincl.lpszClassName = INTERNAL_WINDOW_NAME;
      wincl.lpfnWndProc = childWndProc, //&processWndEvent;
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

    // static LRESULT processWndEvent( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
    // {
    //   return DefWindowProc(hwnd, msg, wParam, lParam);
    // }

   static inline const auto INTERNAL_WINDOW_NAME = L"nxInternalWindow";
   static inline const auto WINDOW_NAME = L"nxChildWindow";

   // we only need to register a window once
   static inline bool m_isRegistered { false };
  };

}