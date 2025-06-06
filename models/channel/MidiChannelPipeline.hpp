#pragma once

// #include "models/Interfaces.hpp"

#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "models/data/PipelineContext.hpp"
#include "models/ParticleLayoutManager.hpp"
#include "models/channel/ChannelPipeline.hpp"

namespace nx
{

  class MidiChannelPipeline final : public ChannelPipeline
  {

  public:

    MidiChannelPipeline( PipelineContext& context, const int32_t channelId );
    ~MidiChannelPipeline() override = default;

    nlohmann::json saveChannelPipeline() const override;

    void loadChannelPipeline( const nlohmann::json& j ) override;

    void processMidiEvent( const Midi_t& midiEvent ) const;

    void update( const sf::Time& deltaTime ) const override;

    void drawMenu() override;

  };
}