#pragma once

#include "models/IAudioVisualizer.hpp"
#include "models/data/PipelineContext.hpp"
// #include "models/shader/BlenderShader.hpp"

namespace nx
{
  class PlotLinesVisualizer final : public IAudioVisualizer
  {

  public:

    explicit PlotLinesVisualizer( PipelineContext& ctx )
      : m_ctx( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_AudioVisualizerType getType() const override
    {
      return E_AudioVisualizerType::E_PlotLineVisualizer;
    }

    void destroyTextures() override
    {
      m_texture.destroy();
    }

    void receiveUpdatedAudioBuffer(const AudioDataBuffer &fft) override
    {
      m_fftBins = fft;

      // Get time since last frame
      const auto delta = ImGui::GetIO().DeltaTime;

      for (size_t i = 0; i < FFT_BINS; ++i)
      {
        float target = m_fftBins[i];
        float& current = m_smoothedBins[i];

        if (target > current)
        {
          current = target; // rise instantly
        }
        else
        {
          const float decayAmount = m_decayRate * delta;
          current = std::max(current - decayAmount, target);
        }
      }
    }

    sf::BlendMode & getBlendMode() override { return m_blend; }

    bool isEnabled() const override { return true; }
    void setEnabled(bool value) override {}

    sf::RenderTexture * draw(sf::RenderTexture * inTexture) override
    {
      m_texture.ensureSize( m_ctx.globalInfo.windowSize );

      return m_texture.get();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Visualizer: PlotLines") )
      {
        ImGui::SliderFloat("Max Y", &m_yScale, 0.01f, 2.0f, "%.2f");
        ImGui::SliderInt("Label Step (bins)", &m_binStep, 1, FFT_BINS / 2);

        ImGui::SliderFloat( "Plot Width", &m_plotWidth, 50.f, 600.0f, "Width %.2f" );
        ImGui::SliderFloat( "Plot Height", &m_plotHeight, 50.f, 600.0f, "Height %.2f" );

        ImGui::SliderFloat( "x-Label Offset", &m_xLabelOffset, -8.0f, 8.0f, "%.2f" );

        ImGui::SliderFloat( "Decay Rate##1", &m_decayRate, 0.01f, 10.0f, "%.2f" );

        ImGui::SeparatorText( "Test Output" );
        ImGui::PlotLines(
            "Bins",
            m_smoothedBins.data(),
            static_cast<int>(m_smoothedBins.size()),
            0,
            nullptr,
            0.0f,
            m_yScale,
            ImVec2(m_plotWidth, m_plotHeight));

        // Draw frequency bin labels below the plot
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 plotPos = ImGui::GetCursorScreenPos();

        const float labelY = plotPos.y + m_xLabelOffset; // A little below the plot

        for ( int32_t i = 0; i < static_cast<int32_t>(m_fftBins.size()); i += m_binStep )
        {
          const float x = plotPos.x + (i / static_cast< float >(m_fftBins.size())) * m_plotWidth;
          const float freq = i * m_ctx.globalInfo.sampleRate / static_cast< float >(FFT_SIZE);

          char buf[32];
          if (freq >= 1000)
            snprintf(buf, sizeof(buf), "%.1fk", freq / 1000.f);
          else
            snprintf(buf, sizeof(buf), "%.0f", freq);

          drawList->AddText(ImVec2(x, labelY), IM_COL32(255, 255, 255, 180), buf);
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update(const sf::Time&) override
    {}

  private:
    PipelineContext& m_ctx;
    LazyTexture m_texture;
    sf::BlendMode m_blend { sf::BlendAdd };
    AudioDataBuffer m_fftBins {};
    AudioDataBuffer m_smoothedBins {};   // Visual output buffer
    float m_yScale { 1.05f };
    int32_t m_binStep { 32 };

    float m_plotWidth { 600.f };
    float m_plotHeight { 100.f };

    float m_xLabelOffset { -8.f };
    float m_decayRate { 3.7f };
  };
}