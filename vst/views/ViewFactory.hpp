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

#ifdef WIN32

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