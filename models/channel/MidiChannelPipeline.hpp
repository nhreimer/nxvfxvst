#pragma once

#include "models/data/PipelineContext.hpp"
#include "models/channel/ChannelPipeline.hpp"

namespace nx
{

  struct Midi_t;

  class MidiChannelPipeline final : public ChannelPipeline
  {

  public:

    MidiChannelPipeline( PipelineContext& context, const int32_t channelId );
    ~MidiChannelPipeline() override = default;

    void processMidiEvent( const Midi_t& midiEvent ) const;

    void update( const sf::Time& deltaTime ) const override;

    void drawMenu() override;

  };
}