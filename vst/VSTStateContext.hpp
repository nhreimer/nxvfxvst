#pragma once
#include <nlohmann/json_fwd.hpp>


#include "vst/params/VSTParamBindingManager.hpp"

namespace nx
{
  ///
  /// The VST State Context shares state with new IVSTView instantiations
  /// because the View has a shorter lifespan than the plugin, e.g., the plugin
  /// window can be closed, which destroys the view, but the plugin is still active.
  /// When the user opens the active plugin window again, a new IVSTView is instantiated.
  struct VSTStateContext
  {
    VSTParamBindingManager * paramBindingManager;
    nlohmann::json * state;
  };
}