// TessellatedGridLayout.hpp
#pragma once

#include "models/audio/IFFTResult.hpp"
#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

namespace layout::tessellatedGrid
{
#define TESSELLATED_GRID_VISUALIZER_PARAMS(X)                                                   \
X(rows,      int32_t, 8,    1,   32,  "Number of rows in the grid",                true)        \
X(cols,      int32_t, 16,   1,   64,  "Number of columns in the grid",             true)        \
X(gain,      float,   1.f,  0.f, 100.f, "Multiplier for intensity/reactivity",     true)        \
X(threshold, float,   0.1f, 0.f, 1.f,  "Minimum value before triggering response", true)

  struct TessellatedGridVisualizerData_t
  {
    bool isActive = true;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(TESSELLATED_GRID_VISUALIZER_PARAMS)
  };

  enum class E_TessellatedGridParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(TESSELLATED_GRID_VISUALIZER_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_TessellatedGridParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(TESSELLATED_GRID_VISUALIZER_PARAMS)
  };
}

  class TessellatedGridVisualizer final
    : public ParticleLayoutBase< layout::tessellatedGrid::TessellatedGridVisualizerData_t >
  {
  public:

    explicit TessellatedGridVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {
      EXPAND_SHADER_VST_BINDINGS(TESSELLATED_GRID_VISUALIZER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j =
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };

      EXPAND_SHADER_PARAMS_TO_JSON(TESSELLATED_GRID_VISUALIZER_PARAMS)

      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(TESSELLATED_GRID_VISUALIZER_PARAMS)
      }

      if ( j.contains( "particleGenerator" ) )
        m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_TessellationVisualizer; }

    void processAudioBuffer(const IFFTResult& fftResult) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto& size = m_ctx.globalInfo.windowSize;

      const float cellW = static_cast< float >(size.x) / static_cast<float>(m_data.cols.first);
      const float cellH = static_cast< float >(size.y) / static_cast<float>(m_data.rows.first);
      const int totalCells = m_data.rows.first * m_data.cols.first;

      for (int row = 0; row < m_data.rows.first; ++row)
      {
        for (int col = 0; col < m_data.cols.first; ++col)
        {
          const int cellIdx = row * m_data.cols.first + col;
          const int binIdx = (cellIdx * FFT_BINS) / totalCells;

          if (binIdx >= FFT_BINS)
            continue;

          const float mag = fft[binIdx] * m_data.gain.first;

          if (mag < m_data.threshold.first)
            continue;

          const float recentMax = m_recentMaxEnergy.updateMaxEnergy( mag );

          const float normMag = std::clamp(mag / (recentMax + 1e-5f), 0.f, 1.f);
          const float eased = Easings::easeOutExpo(normMag);

          const sf::Vector2f pos =
          {
            static_cast< float >(col) * cellW + 0.5f * cellW,
            static_cast< float >(row) * cellH + 0.5f * cellH
          };

          auto * p = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(
              eased, m_ctx.globalInfo.elapsedTimeSeconds));

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

        EXPAND_SHADER_IMGUI(TESSELLATED_GRID_VISUALIZER_PARAMS, m_data)

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    layout::tessellatedGrid::TessellatedGridVisualizerData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMaxEnergy;
  };
}