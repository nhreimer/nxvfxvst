#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstevents.h"
#include <nlohmann/json.hpp>

namespace nx
{
  struct IVSTView : public Steinberg::IPlugView
  {
    virtual ~IVSTView() = default;
    virtual void notify( Steinberg::Vst::Event& event ) = 0;
    virtual void saveState( nlohmann::json& j ) = 0;
    virtual void restoreState( nlohmann::json& j ) = 0;
  };
}