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

//------------------------------------------------------------------------
// Copyright(c) 2025 nx.
//------------------------------------------------------------------------

#include "myplugincontroller.h"
#include "myplugincids.h"
#include "pluginterfaces/base/ustring.h"

#include "vst/views/ViewFactory.hpp"


using namespace Steinberg;

namespace nx {

//------------------------------------------------------------------------
// nxvfxvstController Implementation
//------------------------------------------------------------------------

tresult PLUGIN_API nxvfxvstController::notify( Steinberg::Vst::IMessage * message )
{
  tresult result = kResultOk;

  // do not process message while UI is inactive
  if ( !m_isViewActive || !m_ptrView ) return Steinberg::kResultFalse;

  if ( !message ) return Steinberg::kInvalidArgument;

  if ( Steinberg::FIDStringsEqual( message->getMessageID(), "midi" ) )
  {
    const void * ptrData = nullptr;
    Steinberg::uint32 msgSize = 0;
    message->getAttributes()->getBinary( "onEvent", ptrData, msgSize );

    if ( msgSize > 0 )
    {
      auto event = *( ( Vst::Event * )ptrData );
      m_ptrView->notify( event );
    }
    else
    {
      result = kResultFalse;
      LOG_WARN( "received empty message" );
    }
  }
  else if ( Steinberg::FIDStringsEqual( message->getMessageID(), "bpm" ) )
  {
    if ( message->getAttributes()->getFloat( "bpm", m_lastBPM ) == kResultOk )
    {
      m_ptrView->notifyBPMChange( m_lastBPM );
    }
    else
    {
      LOG_ERROR( "BPM notification failed" );
    }
  }
  else if ( Steinberg::FIDStringsEqual( message->getMessageID(), "playhead" ) )
  {
    if ( message->getAttributes()->getFloat( "playhead", m_lastPlayhead ) == kResultOk )
    {
      m_ptrView->notifyPlayheadUpdate( m_lastPlayhead );
    }
    else
    {
      LOG_ERROR( "Playhead notification failed" );
    }
  }
  else if ( Steinberg::FIDStringsEqual( message->getMessageID(), "sampleRate" ) )
  {
    if ( message->getAttributes()->getFloat( "sampleRate", m_sampleRate ) == kResultOk )
    {
      m_ptrView->notifySampleRate( m_sampleRate );
    }
    else
    {
      LOG_ERROR( "Playhead notification failed" );
    }
  }
  else
  {
    // allow the view to have raw message passthroughs, e.g., for Audio Data.
    // this is because there might be buffers of data that the controller class really
    // shouldn't be responsible for
    m_ptrView->notify( message );
  }

  return result;

}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API nxvfxvstController::connect( IConnectionPoint* other )
{
  LOG_INFO( "processor connected to controller" );
  return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API nxvfxvstController::disconnect( IConnectionPoint* other )
{
  LOG_INFO( "processor disconnected from controller" );
  return Steinberg::kResultOk;
}

tresult PLUGIN_API nxvfxvstController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
#ifdef DEBUG
  nx::SLog::initializeFileWriter( NX_LOG_FILE );
  nx::SLog::log->flush_on( spdlog:: level::trace );
#endif

  LOG_INFO( "initializing controller" );

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Register a bunch of dummy parameters, because according to the documentation:
  // Steinberg VST3 SDK Documentation: “All parameters must be created in initialize()
  // to ensure proper parameter registration and automation handling by the host.”
  // on the plus side, we can change the name of the parameter
  for ( int32_t i = 0; i < PARAMETERS_ENABLED; ++i )
  {
    std::string paramName( "Param_" + std::to_string(i) );

    // hands off ownership: this is NOT leaked
    parameters.addParameter(
      new Vst::RangeParameter(USTRING( paramName.c_str() ), i, nullptr, 0.f, 1.f, 0.f));
  }

  LOG_INFO( "initialized controller with {} parameters", PARAMETERS_ENABLED );
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstController::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API nxvfxvstController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
    LOG_DEBUG( "creating view controller" );

	  // save state whenever the view closes
	  m_ptrView = ViewFactory::createView(
	    m_stateContext,
	    [&] ( IVSTView* view )
	    {
	      view->saveState( m_state );
	      m_hasState = true;
	      m_isViewActive = false;
	      LOG_DEBUG( "saved view state. view disabled. note consumption disabled." );
	    } );

	  // restore any state
    if ( m_hasState )
    {
      m_ptrView->restoreState( m_state );
      LOG_DEBUG( "restored view state" );
    }

    m_isViewActive = m_ptrView != nullptr;

	  return m_ptrView;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace nx
