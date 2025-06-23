#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "vst/views/IVSTView.hpp"

#if defined WIN32
#include "vst/views/Win32TimerQueueView.hpp"
#elif defined LINUX || __linux__
#include "vst/views/X11View.hpp"
#endif

namespace nx
{

  struct VSTStateContext;

  class ViewFactory
  {
    public:

      static IVSTView * createView( VSTStateContext& stateContext,
                                    std::function< void( IVSTView * ) >&& onRemoved )
      {

#if defined WIN32

        return new Win32TimerQueueView( stateContext,
                              { 0, 0, 1280, 768 },
                              std::forward< std::function< void( IVSTView * ) > >( onRemoved ) );

#elif defined LINUX || __linux__

        return new X11View( stateContext,
                            { 0, 0, 1280, 768 },
                            std::forward< std::function< void( IVSTView * ) > >( onRemoved ) );
#else
        LOG_ERROR( "IPlugView::createView() not implemented for this system." );
        return nullptr;
#endif

      }
  };

}