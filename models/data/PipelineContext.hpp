#pragma once

#include "models/data/GlobalInfo_t.hpp"
#include "vst/VSTStateContext.hpp"
#include "vst/params/VSTParamBindingManager.hpp"

namespace nx
{

  ///
  /// used for passing through all components of the pipeline
  struct PipelineContext
  {
    PipelineContext( const GlobalInfo_t& info,
                     VSTStateContext& stateContext )
     : globalInfo( info ),
       vstContext( stateContext )
    {}

    const GlobalInfo_t& globalInfo;
    VSTStateContext& vstContext;
  };

}