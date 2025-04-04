#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstevents.h"

namespace nx
{
  struct IVSTView : public Steinberg::IPlugView
  {
    virtual ~IVSTView() = default;
    virtual void notify( Steinberg::Vst::Event& event ) = 0;
  };
}