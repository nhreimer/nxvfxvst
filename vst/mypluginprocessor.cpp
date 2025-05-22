//------------------------------------------------------------------------
// Copyright(c) 2025 nx.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "pluginterfaces/vst/ivstevents.h"
#include "base/source/fstreamer.h"

#include "vst/analysis/FFTBuffer.hpp"

using namespace Steinberg;

namespace nx {
//------------------------------------------------------------------------
// nxvfxvstProcessor
//------------------------------------------------------------------------
nxvfxvstProcessor::nxvfxvstProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (knxvfxvstControllerUID);
}

//------------------------------------------------------------------------
nxvfxvstProcessor::~nxvfxvstProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::process (Vst::ProcessData& data)
{
  processMidiData( data );
  processAudioData( data );

  return kResultOk;
}

//------------------------------------------------------------------------

void PLUGIN_API nxvfxvstProcessor::processMidiData( Vst::ProcessData& data )
{
  if ( data.processContext != nullptr )
  {
    if ( m_clock.getElapsedTime().asSeconds() > m_messageThrottleInMS )
    {
      const auto currentSample = data.processContext->projectTimeSamples;
      const auto sampleRate = data.processContext->sampleRate;
      const auto currentTimeInSeconds = currentSample / sampleRate;

      if ( m_lastPlayhead != currentTimeInSeconds )
      {
        m_lastPlayhead = currentTimeInSeconds; // gets current time in seconds
        sendPlayheadMessage();
        m_clock.restart();
      }
    }

    m_lastBPM = data.processContext->tempo;
    sendBPMMessage();
  }

  if ( data.inputEvents != nullptr )
  {
    for ( Steinberg::int32 i = 0; i < data.inputEvents->getEventCount(); ++i )
    {
      Steinberg::Vst::Event event {};
      if ( data.inputEvents->getEvent( i, event ) != kResultFalse &&
           event.type == Steinberg::Vst::Event::kNoteOnEvent )
      {
        sendMidiNoteEventMessage( event );
      }
    }
  }
}

//------------------------------------------------------------------------

void PLUGIN_API nxvfxvstProcessor::processAudioData( Vst::ProcessData& data )
{
  // ensure we have audio first
  if ( data.numInputs == 0 || data.inputs[ 0 ].numChannels == 1 )
    return;

  const auto& input = data.inputs[0];

  // convert stereo to mono and push into analyzer
  const float * left = input.channelBuffers32[ 0 ];
  const float * right = ( input.numChannels > 1 ) ? input.channelBuffers32[ 1 ] : nullptr;

  for ( int32 i = 0; i < data.numSamples; ++i )
  {
    const float mono = right ? 0.5f * ( left[ i ] + right[ i ] ) : left[ i ];
    m_audioAnalyzer.pushSample( mono );
  }

  // doesn't send unless the buffer is ready
  sendAudioMessage();
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API nxvfxvstProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

  //------------------------------------------------------------------------
void nxvfxvstProcessor::sendMidiNoteEventMessage( const Vst::Event &event ) const
{
  auto * ptrMsg = allocateMessage();
  if ( ptrMsg == nullptr )
  {
    // LOG_ERROR("failed to allocate message");
    return;
  }

  ptrMsg->setMessageID( "midi" );

  if ( ptrMsg->getAttributes()->setBinary( "onEvent", &event, sizeof( Vst::Event ) ) == kResultOk )
  {
    // don't flood the error log with bogus messages. messages
    // are rejected by the controller whenever the UI is inactive.
    this->sendMessage( ptrMsg );
  }
  // else
  //   LOG_ERROR( "failed to set midi data in message" );

  ptrMsg->release();
}
//------------------------------------------------------------------------

void nxvfxvstProcessor::sendBPMMessage() const
{
  auto * ptrMsg = allocateMessage();
  if ( ptrMsg == nullptr ) return;

  ptrMsg->setMessageID( "bpm" );

  if ( ptrMsg->getAttributes()->setFloat( "bpm", m_lastBPM ) == kResultOk )
    this->sendMessage( ptrMsg );

  ptrMsg->release();
}

//------------------------------------------------------------------------

void nxvfxvstProcessor::sendPlayheadMessage() const
{
  auto * ptrMsg = allocateMessage();
  if ( ptrMsg == nullptr ) return;
  ptrMsg->setMessageID( "playhead" );

  if ( ptrMsg->getAttributes()->setFloat( "playhead", m_lastPlayhead ) == kResultOk )
    this->sendMessage( ptrMsg );

  ptrMsg->release();
}

//------------------------------------------------------------------------

void nxvfxvstProcessor::sendAudioMessage()
{
  // ensure FFT is ready
  if ( !m_audioAnalyzer.isFFTReady() ) return;

  // compute the values
  m_audioAnalyzer.computeFFT( m_bins );

  auto * ptrMsg = allocateMessage();
  if ( ptrMsg == nullptr ) return;

  ptrMsg->setMessageID( "FFTData" );
  if ( ptrMsg->getAttributes()->setBinary(
    "FFTData",
    m_bins.data(),
    ( Steinberg::uint32 )( m_bins.size() * sizeof( float ) ) ) == kResultOk )
  {
    this->sendMessage( ptrMsg );
  }

  ptrMsg->release();
}

//------------------------------------------------------------------------
} // namespace nx
