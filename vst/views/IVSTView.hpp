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