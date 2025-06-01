#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "utils/TaskQueue.hpp"
#include "models/channel/ChannelPipeline.hpp"

#include "models/data/PipelineContext.hpp"
#include "models/ShaderPipeline.hpp"
#include "vst/analysis/FFTBuffer.hpp"

#include "models/IAudioVisualizer.hpp"
#include "models/audio/FFTProcessor.hpp"
//#include "models/audio/PlotlineVisualizer.hpp"
#include "models/audio/RingPlotVisualizer.hpp"

namespace nx
{

  class AudioChannelPipeline final : public ChannelPipeline
  {
  public:
    AudioChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : ChannelPipeline( ctx, channelId ),
        m_shaderPipeline( ctx, *this )
    {
      m_audioVisualizer = std::make_unique< RingPlotVisualizer >( ctx );
    }

    ~AudioChannelPipeline() override = default;

    void requestRenderUpdate() override
    {
      // this comes in on the main thread,
      // but we need to move it to the render thread
      request( [ this ]
      {
        m_outputTexture = m_audioVisualizer->draw( nullptr );

        if ( m_outputTexture )
          m_outputTexture = m_shaderPipeline.draw( m_outputTexture );
      } );
    }

    void requestShutdown() override
    {
      request( [ this ]
      {
        // this asks all other pipelines to shut down
        m_audioVisualizer->destroyTextures();
        m_shaderPipeline.destroyTextures();
      } );
    }

    nlohmann::json saveChannelPipeline() const override
    {
      return {};
    }

    void loadChannelPipeline( const nlohmann::json& j ) override
    {}

    void processAudioBuffer( FFTBuffer& buffer )
    {
      // receive the unscaled audio buffer
      const auto& audioBuffer = buffer.getBuffer();

      // scale the FFT buffer to user-customizable values
      m_scaler.apply( m_ctx.globalInfo.sampleRate, audioBuffer );
      m_audioVisualizer->receiveUpdatedAudioBuffer( m_scaler );
    }

    void update( const sf::Time& deltaTime ) const override
    {
      m_audioVisualizer->update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Audio Data" ) )
      {
        m_scaler.drawMenu();
        m_audioVisualizer->drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      m_shaderPipeline.drawMenu();
    }

  private:

    std::unique_ptr< IAudioVisualizer > m_audioVisualizer;
    ShaderPipeline m_shaderPipeline;

    FFTProcessor m_scaler;
  };

}