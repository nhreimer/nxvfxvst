// TessellatedGridLayout.hpp
#pragma once

#include "models/audio/IFFTResult.hpp"
#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "models/data/ParticleLayoutData_t.hpp"

namespace nx
{
  struct TessellatedGridVisualizerData_t : public ParticleLayoutData_t
  {
    bool isActive = true;
    int32_t rows = 8;
    int32_t cols = 16;
    float gain = 1.f;
    float threshold = 0.1f;
  };

  class TessellatedGridVisualizer final
    : public ParticleLayoutBase< TessellatedGridVisualizerData_t >
  {
  public:

    explicit TessellatedGridVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_TessellationVisualizer; }

    void processAudioBuffer(const IFFTResult& fftResult) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto& size = m_ctx.globalInfo.windowSize;

      const float cellW = size.x / static_cast<float>(m_data.cols);
      const float cellH = size.y / static_cast<float>(m_data.rows);
      const int totalCells = m_data.rows * m_data.cols;

      for (int row = 0; row < m_data.rows; ++row)
      {
        for (int col = 0; col < m_data.cols; ++col)
        {
          const int cellIdx = row * m_data.cols + col;
          const int binIdx = (cellIdx * FFT_BINS) / totalCells;

          if (binIdx >= FFT_BINS)
            continue;

          const float mag = fft[binIdx] * m_data.gain;

          if (mag < m_data.threshold)
            continue;

          const float recentMax = m_recentMaxEnergy.updateMaxEnergy( mag );

          const float normMag = std::clamp(mag / (recentMax + 1e-5f), 0.f, 1.f);
          const float eased = Easings::easeOutExpo(normMag);

          const sf::Vector2f pos =
          {
            col * cellW + 0.5f * cellW,
            row * cellH + 0.5f * cellH
          };

          auto * p = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(eased, m_ctx.globalInfo.elapsedTimeSeconds));

          p->setPosition(pos);
          ParticleLayoutBase::notifyBehaviorOnSpawn(p);
        }
      }

      m_timedBuffer.stopTimerAndAddSample();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Tessellation Layout" ) )
      {
        m_particleGeneratorManager.drawMenu();
        ImGui::Separator();

        m_particleGeneratorManager.getParticleGenerator()->drawMenu();
        ImGui::SeparatorText( "Tessellation Layout Options" );

        ImGui::SliderInt("Grid Rows", &m_data.rows, 1, 32);
        ImGui::SliderInt("Grid Cols", &m_data.cols, 1, 64);
        ImGui::SliderFloat("Gain", &m_data.gain, 1.f, 100.f);
        ImGui::SliderFloat("Threshold", &m_data.threshold, 0.f, 1.f);

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    TessellatedGridVisualizerData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMaxEnergy;
  };
}