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
#include "pluginterfaces/vst/ivstevents.h"
#include <nlohmann/json.hpp>

namespace nx
{
  struct IVSTView : public Steinberg::IPlugView
  {
    virtual ~IVSTView() = default;

    // Generic and specific messages defined here
    virtual void notify( Steinberg::Vst::IMessage * rawMsg ) = 0;
    virtual void notify( Steinberg::Vst::Event& event ) = 0;
    virtual void notifyBPMChange( double bpm ) = 0;
    virtual void notifyPlayheadUpdate( double playhead ) = 0;
    virtual void notifySampleRate( double sampleRate ) = 0;

    virtual void saveState( nlohmann::json& j ) = 0;
    virtual void restoreState( nlohmann::json& j ) = 0;

    virtual void setFrameRate( int32_t fps ) = 0;
    virtual int32_t getFrameRate() const = 0;
  };
}