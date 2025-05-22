#pragma once

#include <concurrentqueue/concurrentqueue.h>

#include "imgui-SFML.h"

#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/base/keycodes.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include "../models/channel/MidiChannelPipeline.hpp"
#include "models/MultichannelPipeline.hpp"
#include "models/data/GlobalInfo_t.hpp"
#include "models/data/PipelineContext.hpp"

#include "vst/VSTStateContext.hpp"

#include "vst/analysis/FFTBuffer.hpp"

namespace nx
{

  // Event Facade for the VST3 plugin
  class EventFacadeVst final
  {
  public:

    explicit EventFacadeVst( VSTStateContext& stateContext );

    ~EventFacadeVst() = default;

    void onKeyDown(Steinberg::char16 key, Steinberg::int16 keyCode, Steinberg::int16 modifiers);
    void onKeyUp( Steinberg::char16 key, Steinberg::int16 keyCode, Steinberg::int16 modifiers );

    void saveState( nlohmann::json &j ) const;

    void restoreState( nlohmann::json &j );

    void processVstEvent( Steinberg::Vst::IMessage * rawMessage );

    void processVstEvent( const Steinberg::Vst::Event & event );

    void processBPMChange( const double bpm );

    void processPlayheadUpdate( const double playhead );

    void initialize( sf::RenderWindow & window );

    void shutdown( sf::RenderWindow & window );

    bool executeFrame( sf::RenderWindow & window );

    void onResize( sf::RenderWindow & window, uint32_t width, uint32_t height );

  private:

    void drawMenu();
    void consumeMidiEvents();
    void consumeAudioData();

  private:

    // this is the original, non-const verison that the Pipeline Context uses
    GlobalInfo_t m_globalInfo;

    // this is what gets handed off to all the components
    PipelineContext m_pipelineContext;
    MultichannelPipeline m_pipelines;

    sf::Clock m_clock;

    // used for ensuring that ImGui doesn't lock whenever the focus is lost
    sf::Clock m_focusTimer;

    // receives midi events on the processor thread
    // and processes them on the controller thread
    //Concurrency::concurrent_queue< Midi_t > m_queue;
    moodycamel::ConcurrentQueue< Midi_t > m_queue;

    // buffer with audio data for audio graphics
    FFTBuffer m_fftBuffer;
  };

}