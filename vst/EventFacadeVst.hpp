#pragma once

#include <concurrent_queue.h>

#include <SFML/Graphics/RenderWindow.hpp>

#include "imgui-SFML.h"

#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/base/keycodes.h"

#include "models/data/GlobalInfo_t.hpp"
#include "models/ChannelPipeline.hpp"
#include "models/MultichannelPipeline.hpp"
#include "models/data/PipelineContext.hpp"

#include "vst/VSTStateContext.hpp"

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

    void processVstEvent( const Steinberg::Vst::Event & event );

    void processBPMChange( const double bpm );

    void initialize( sf::RenderWindow & window );

    void shutdown( sf::RenderWindow & window );

    bool executeFrame( sf::RenderWindow & window );

    void onResize( sf::RenderWindow & window, uint32_t width, uint32_t height );

  private:

    void drawMenu();
    void consumeMidiEvents();

  private:

    // this is the original, non-const verison that the Pipeline Context uses
    GlobalInfo_t m_globalInfo;

    // this is what gets handed off to all the components
    PipelineContext m_pipelineContext;
    MultichannelPipeline m_pipelines;

    sf::Clock m_clock;

    // receives midi events on the processor thread
    // and processes them on the controller thread
    Concurrency::concurrent_queue< Midi_t > m_queue;
  };

}