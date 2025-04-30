#pragma once

#include "models/Interfaces.hpp"

#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "data/PipelineContext.hpp"
#include "models/ParticleLayoutManager.hpp"

#include "helpers/Definitions.hpp"

namespace nx
{
  class ChannelPipeline final
  {

  public:

    ChannelPipeline( PipelineContext& context, const int32_t channelId );
    ~ChannelPipeline() = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const;

    void loadChannelPipeline( const nlohmann::json& j );

    void processMidiEvent( const Midi_t& midiEvent ) const;

    // this is not hooked up to anything. could be cool for mouse-based effects or so
    void processEvent( const sf::Event &event ) const {}

    void update( const sf::Time& deltaTime ) const;

    const sf::RenderTexture& draw();

    void drawMenu();

    int32_t getDrawPriority() const { return m_drawPriority; }
    const sf::BlendMode& getChannelBlendMode() const { return m_blendMode; }

  private:

    void drawChannelPipelineMenu();

  private:

    PipelineContext& m_ctx;

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    ParticleLayoutManager m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    bool m_isBypassed { false };

    // defaults to the channel ID
    int32_t m_drawPriority;

    inline static std::array< std::string, MAX_CHANNELS > m_drawPriorityNames;
  };
}