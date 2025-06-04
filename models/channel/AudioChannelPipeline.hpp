#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "utils/TaskQueue.hpp"
#include "vst/analysis/FFTBuffer.hpp"

#include "models/channel/ChannelPipeline.hpp"

#include "models/data/PipelineContext.hpp"
#include "models/ShaderPipeline.hpp"

#include "models/IAudioVisualizer.hpp"

#include "models/audio/FFTProcessor.hpp"

#include "models/audio/RingPlotVisualizer.hpp"
#include "models/audio/RingParticleVisualizer.hpp"

namespace nx
{
  class AudioChannelPipeline final : public ChannelPipeline
  {
  public:
    AudioChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : ChannelPipeline( ctx, channelId ),
        m_shaderPipeline( ctx, *this )
    {
      m_audioVisualizer = std::make_unique< RingParticleVisualizer >( ctx );
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
      m_shaderPipeline.processAudioBuffer( audioBuffer );
    }

    void update( const sf::Time& deltaTime ) const override
    {
      m_audioVisualizer->update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void drawMenu() override
    {
      m_scaler.drawMenu();
      drawVisualizerOptionsMenu();
      m_audioVisualizer->drawMenu();
      m_shaderPipeline.drawMenu();
    }

  private:

    // because the IAudioVisualizer implementation contains textures, we
    // have to make sure we destroy the texture on the render thread.
    template < typename T >
    void requestAndReplaceVisualizer()
    {
      request( [ this ]
      {
        std::unique_lock lock( m_audioVisualizerMutex );
        m_audioVisualizer->destroyTextures();
        m_audioVisualizer.reset( new T( m_ctx ) );
      } );
    }

    void drawVisualizerOptionsMenu()
    {
      if ( ImGui::TreeNode( "Audio Visualizers Available" ) )
      {
        if ( ImGui::RadioButton( "Ring Particles##1", E_AudioVisualizerType::E_RingParticleVisualizer == m_audioVisualizer->getType() ) )
          requestAndReplaceVisualizer< RingParticleVisualizer >();
        if ( ImGui::RadioButton( "Ring Plot##1", E_AudioVisualizerType::E_RingPlotVisualizer == m_audioVisualizer->getType() ) )
          requestAndReplaceVisualizer< RingPlotVisualizer >();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    std::unique_ptr< IAudioVisualizer > m_audioVisualizer;
    ShaderPipeline m_shaderPipeline;

    FFTProcessor m_scaler;

    std::mutex m_audioVisualizerMutex;
  };

}