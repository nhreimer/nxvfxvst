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

	// Register a bunch of dummy parameters
  for (int32_t i = 0; i < 128; ++i)
  {
    std::string paramName( "Param_" + std::to_string(i) );

    // hands off ownership
    auto * param = new Vst::RangeParameter(USTRING( paramName.c_str() ), i, nullptr, 0.f, 1.f, 0.f);
    parameters.addParameter(param);
  }

  LOG_INFO( "initialized controller with 128 parameters" );
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
