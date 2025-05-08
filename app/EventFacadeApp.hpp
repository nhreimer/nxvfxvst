#pragma once

#include <imgui-SFML.h>

#include <concurrentqueue/concurrentqueue.h>
#include <pluginterfaces/vst/ivstevents.h>

#include "models/data/Midi_t.hpp"
#include "models/data/GlobalInfo_t.hpp"
#include "models/MultichannelPipeline.hpp"

#include "models/data/PipelineContext.hpp"
#include "MidiGenerator.hpp"

namespace nx
{

  // Event Facade for the standalone, i.e., non-plugin version
  class EventFacadeApp final
  {
  public:

    EventFacadeApp();

    void processVstEvent( const Steinberg::Vst::Event & event );

    void initialize( sf::RenderWindow & window );

    void shutdown( const sf::RenderWindow & window );

    // gets called whenever the OS indicates it's time to update
    bool executeFrame( sf::RenderWindow & window );

    void onResize( sf::RenderWindow & window, uint32_t width, uint32_t height );

  private:

    void drawMenu();

    void consumeMidiEvents();

    // ONLY exists in the standalone for debugging purposes
    static void drawDebugOverlay(const sf::RenderWindow& window);

  private:

    bool m_midiGenIsRunning { false };

    const std::function< void( const Steinberg::Vst::Event& ) > m_onEvent
    {
      [ & ]( const Steinberg::Vst::Event & event )
      {
        processVstEvent( event );
      }
    };

    std::array< test::MidiGenerator, 4 > m_midiGen {
      test::MidiGenerator { 0, 1000, m_onEvent },
      test::MidiGenerator { 1, 100, m_onEvent },
      test::MidiGenerator { 2, 5000, m_onEvent },
      test::MidiGenerator { 3, 500, m_onEvent } };

    // the following two are dummy vars for the app since there is no automation to perform
    VSTParamBindingManager m_paramBindingManager
    {
      []( const int32_t paramId, const float normalizedValue ) {},
      []( const int32_t paramId ) {}
    };

    VSTStateContext m_stateContext { m_paramBindingManager };

    GlobalInfo_t m_globalInfo
    {
      .bpm = 120.f
    };

    // this is what gets handed off to all the components
    PipelineContext m_ctx;

    sf::Clock m_timer;

    // used for ensuring that ImGui doesn't lock whenever the focus is lost
    sf::Clock m_focusTimer;

    // only one right now
    MultichannelPipeline m_pipelines;

    // receives midi events on the processor thread
    // and processes them on the controller thread
    moodycamel::ConcurrentQueue< Midi_t > m_queue;

  };

}