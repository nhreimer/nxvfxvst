//------------------------------------------------------------------------
// Copyright(c) 2025 nx.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "helpers/Definitions.hpp"
#include "vst/analysis/AudioAnalyzer.hpp"

// forward decl
namespace Steinberg::Vst { struct Event; }

namespace nx
{

//------------------------------------------------------------------------
//  nxvfxvstProcessor
//------------------------------------------------------------------------
class nxvfxvstProcessor : public Steinberg::Vst::AudioEffect
{
public:
	nxvfxvstProcessor ();
	~nxvfxvstProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return static_cast< Steinberg::Vst::IAudioProcessor * >( new nxvfxvstProcessor );
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:

private:

  void processMidiData( Steinberg::Vst::ProcessData& data );
  void processAudioData( Steinberg::Vst::ProcessData& data );
  void passThroughAudio( Steinberg::Vst::ProcessData& data );

  void sendMidiNoteEventMessage( const Steinberg::Vst::Event& event ) const;
  void sendAudioMessage();
  void sendFloatMessage( const std::string& messageId, const double value ) const;

private:

  double m_lastPlayhead { 0.f };
  double m_lastBPM { 0.f };
  sf::Clock m_clock;
  AudioAnalyzer m_audioAnalyzer;
  AudioDataBuffer m_bins;

  static constexpr double m_messageThrottleInMS = PLAYHEAD_INTERVAL_IN_SECS;
};

//------------------------------------------------------------------------
} // namespace nx
