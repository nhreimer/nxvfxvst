//------------------------------------------------------------------------
// Copyright(c) 2025 nx.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "vst/views/ViewFactory.hpp"

namespace nx {

//------------------------------------------------------------------------
//  nxvfxvstController
//------------------------------------------------------------------------
class nxvfxvstController : public Steinberg::Vst::EditControllerEx1
{
public:
//------------------------------------------------------------------------
	nxvfxvstController() { m_state = nlohmann::json::object(); }
	~nxvfxvstController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new nxvfxvstController;
	}

  // --- message passing interface ---
  Steinberg::tresult PLUGIN_API notify( Steinberg::Vst::IMessage * message ) SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API connect( IConnectionPoint* other ) SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API disconnect( IConnectionPoint* other ) SMTG_OVERRIDE;

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:

private:
  IVSTView * m_ptrView { nullptr };
  bool m_hasState { false };
  bool m_isViewActive { false };

  double m_lastBPM { 0.f };

  // used between closing and opening the window
  nlohmann::json m_state;
};

//------------------------------------------------------------------------
} // namespace nx
