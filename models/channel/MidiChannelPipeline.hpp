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
    //
    // const sf::RenderTexture& draw();

    void drawMenu() override;

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

  private:

    void drawChannelPipelineMenu();

  private:

    ParticleLayoutManager m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

  };
}