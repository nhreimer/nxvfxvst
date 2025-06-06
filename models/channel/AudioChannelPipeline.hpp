#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "utils/TaskQueue.hpp"
#include "vst/analysis/FFTBuffer.hpp"

#include "models/channel/ChannelPipeline.hpp"

#include "models/data/PipelineContext.hpp"

#include "models/audio/FFTProcessor.hpp"

namespace nx
{
  class AudioChannelPipeline final : public ChannelPipeline
  {
  public:
    AudioChannelPipeline( PipelineContext& ctx, const int32_t channelId )
      : ChannelPipeline( ctx, channelId )
    {}

    ~AudioChannelPipeline() override = default;


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
      m_particleLayout.processAudioBuffer( m_scaler );
      m_shaderPipeline.processAudioBuffer( audioBuffer );
    }

    void update( const sf::Time& deltaTime ) const override
    {
      m_particleLayout.update( deltaTime );
      m_modifierPipeline.update( deltaTime );
      m_shaderPipeline.update( deltaTime );
    }

    void drawMenu() override
    {
      m_scaler.drawMenu();
      m_particleLayout.drawAudioMenu();

      ImGui::Separator();
      m_modifierPipeline.drawMenu();

      ImGui::Separator();
      m_shaderPipeline.drawMenu();

      ImGui::Separator();
      drawChannelPipelineMenu();
      ImGui::Separator();
    }

  private:

    FFTProcessor m_scaler;
    std::mutex m_audioVisualizerMutex;
  };

}