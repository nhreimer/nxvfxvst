#pragma once

#include "models/Interfaces.hpp"

#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "data/PipelineContext.hpp"
#include "models/ParticleLayoutManager.hpp"

#include "helpers/Definitions.hpp"

#include "utils/TaskQueue.hpp"

namespace nx
{
  class ChannelPipeline final : public TaskQueueRequestSink
  {

  public:

    ChannelPipeline( PipelineContext& context, const int32_t channelId );
    ~ChannelPipeline() override = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const;

    void loadChannelPipeline( const nlohmann::json& j );

    void processMidiEvent( const Midi_t& midiEvent ) const;

    // this is not hooked up to anything. could be cool for mouse-based effects or so
    void processEvent( const sf::Event &event ) const {}

    void update( const sf::Time& deltaTime ) const;
    //
    // const sf::RenderTexture& draw();

    void drawMenu();

    void requestShutdown() override
    {
      request( [ this ]
      {
        // this asks all other pipelines to shut down
        m_modifierPipeline.destroyTextures();
        m_shaderPipeline.destroyTextures();
      } );
    }

    void requestRenderUpdate() override
    {
      // this comes in on the main thread,
      // but we need to move it to the render thread
      request( [ this ]
      {
        const auto * modifierTexture = m_modifierPipeline.applyModifiers(
          m_particleLayout.getParticleOptions(),
          m_particleLayout.getParticles() );

        m_outputTexture = m_shaderPipeline.draw( modifierTexture );
      } );
    }

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

    sf::RenderTexture * m_outputTexture { nullptr };

    bool m_isBypassed { false };

    // defaults to the channel ID
    int32_t m_drawPriority;

    inline static std::array< std::string, MAX_CHANNELS > m_drawPriorityNames;
  };
}