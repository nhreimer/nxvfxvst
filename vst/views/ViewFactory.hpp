#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "vst/views/IVSTView.hpp"

#ifdef WIN32
#include "vst/views/Win32View.hpp"
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

#ifdef WIN32

        return new Win32View( stateContext,
                              { 0, 0, 1280, 768 },
                              std::forward< std::function< void( IVSTView * ) > >( onRemoved ) );

#else
        LOG_ERROR( "IPlugView::createView() not implemented for this system." );
        return nullptr;
#endif

      }
  };

}