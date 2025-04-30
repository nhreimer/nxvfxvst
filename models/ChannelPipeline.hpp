#pragma once

#include "models/Interfaces.hpp"

#include "models/data/Midi_t.hpp"

#include "models/ModifierPipeline.hpp"
#include "models/ShaderPipeline.hpp"

#include "data/PipelineContext.hpp"
#include "models/ParticleLayoutManager.hpp"

#include <future>

namespace nx
{
  class ChannelPipeline final
  {

  public:

    explicit ChannelPipeline( PipelineContext& context )
      : m_ctx( context ),
        m_particleLayout( context ),
        m_modifierPipeline( context ),
        m_shaderPipeline( context )
    {}

    ~ChannelPipeline() = default;

    void toggleBypass() { m_isBypassed = !m_isBypassed; }
    bool isBypassed() const { return m_isBypassed; }

    nlohmann::json saveChannelPipeline() const;

    void loadChannelPipeline( const nlohmann::json& j );

    void processMidiEvent( const Midi_t& midiEvent ) const;

    void processEvent( const sf::Event &event ) const
    {
      // TODO: add? maybe?
    }

    void update( const sf::Time& deltaTime ) const;

    void draw( sf::RenderWindow& window );

    void drawMenu();

  private:

    void drawChannelPipelineMenu();

  private:

    PipelineContext& m_ctx;

    std::atomic< bool > m_isReady { false };

    // the blend mode is important in case there are multiple channel pipelines
    sf::BlendMode m_blendMode { sf::BlendAdd };

    ParticleLayoutManager m_particleLayout;
    ModifierPipeline m_modifierPipeline;
    ShaderPipeline m_shaderPipeline;

    bool m_isBypassed { false };
  };
}