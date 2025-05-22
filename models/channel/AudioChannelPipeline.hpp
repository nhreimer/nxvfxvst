#pragma once

#include "utils/TaskQueue.hpp"
#include "models/channel/ChannelPipeline.hpp"

#include "models/data/PipelineContext.hpp"
#include "models/ShaderPipeline.hpp"
#include "vst/analysis/FFTBuffer.hpp"

namespace nx
{

  class AudioChannelPipeline final : public ChannelPipeline
  {
  public:
    AudioChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : ChannelPipeline( ctx, channelId ),
        m_shaderPipeline( ctx, *this )
    {}

    ~AudioChannelPipeline() override = default;

    void requestRenderUpdate() override
    {
      // this comes in on the main thread,
      // but we need to move it to the render thread
      request( [ this ]
      {
        // const auto * modifierTexture = m_modifierPipeline.applyModifiers(
        //   m_particleLayout.getParticleOptions(),
        //   m_particleLayout.getParticles() );
        //
        // m_outputTexture = m_shaderPipeline.draw( modifierTexture );
      } );
    }

    void requestShutdown() override
    {
      request( [ this ]
      {
        // this asks all other pipelines to shut down
        //m_modifierPipeline.destroyTextures();
        m_shaderPipeline.destroyTextures();
      } );
    }

    nlohmann::json saveChannelPipeline() const override
    {
      return {};
    }

    void loadChannelPipeline( const nlohmann::json& j ) override
    {

    }

    void processAudioBuffer( const FFTBuffer& buffer ) const
    {

    }

    void update( const sf::Time& deltaTime ) const override
    {
      m_shaderPipeline.update( deltaTime );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Audio Data" ) )
      {
        ImGui::Text( "getting audio, man!" );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    ShaderPipeline m_shaderPipeline;

  };

}