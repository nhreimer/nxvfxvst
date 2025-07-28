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
    explicit VSTStateContext( VSTParamBindingManager& bindingManager )
     : paramBindingManager( bindingManager )
    {}

    VSTParamBindingManager& paramBindingManager;
  };
}