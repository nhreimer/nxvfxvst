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

#pragma once

#include <dwmapi.h>

namespace nx::win32
{

  class Win32Helper
  {
  public:

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


    static HWND createChildWindow( LRESULT (*childWndProc)( HWND, UINT, WPARAM, LPARAM ),
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