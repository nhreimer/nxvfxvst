#pragma once

namespace nx
{

namespace layout::plotline
{
#define PLOT_LINE_VISUALIZER_PARAMS(X)                                                                   \
X(gain,             float,   0.0f,   0.0f,   100.0f, "Amplitude multiplier",                true)        \
X(threshold,        float,   0.1f,   0.0f,   1.0f,   "Trigger threshold",                   true)        \
X(heightMod,        float, 200.0f,   0.0f,  10000.0f, "Pixel scale for waveform height",    true)        \
X(enableHarmonics,  bool,    true,   0,      1,      "Enable harmonic layers",              true)        \
X(harmonicCount,    int,        3,   1,     10,      "Number of harmonics to add",          true)        \
X(harmonicFalloff,  float,   0.5f,   0.0f,   1.0f,   "Intensity falloff per harmonic",      true)        \
X(enableMirroring,  bool,    true,   0,      1,      "Mirror waveform across horizontal",   true)

  struct PlotLineVisualizerData_t
  {
    bool isActive = true;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(PLOT_LINE_VISUALIZER_PARAMS)
  };

  enum class E_PlitLineParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(PLOT_LINE_VISUALIZER_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_PlitLineParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(PLOT_LINE_VISUALIZER_PARAMS)
  };
}

  class PlotLineVisualizer final
    : public ParticleLayoutBase< layout::plotline::PlotLineVisualizerData_t >
  {
  public:

    explicit PlotLineVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j =
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };

      EXPAND_SHADER_PARAMS_TO_JSON(PLOT_LINE_VISUALIZER_PARAMS)

      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(PLOT_LINE_VISUALIZER_PARAMS)
      }

      if ( j.contains( "particleGenerator" ) )
        m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

      if ( j.contains( "easing" ) )
        m_easing.deserialize( j.at( "easing" ) );
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_PlotLineVisualizer; }

    void processAudioBuffer(const IFFTResult& fftResult) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto size = m_ctx.globalInfo.windowSize;
      const float binWidth = static_cast<float>(size.x) / static_cast<float>(FFT_BINS);

      for (size_t i = 0; i < FFT_BINS; ++i)
      {
        float energy = fft[i];

        // --- Multi-octave enhancement ---
        if (m_data.enableHarmonics.first)
        {
          for (int h = 2; h <= m_data.harmonicCount.first; ++h)
          {
            const size_t harmonicIndex = i * h;
            if (harmonicIndex < FFT_BINS)
              energy += fft[harmonicIndex] * m_data.harmonicFalloff.first;
            else
              break;
          }
        }

        if (energy < m_data.threshold.first)
          continue;

        const float recentMax = m_recentMax.updateMaxEnergy( energy );

        // Normalize, apply gain and easing
        const float norm = std::clamp(energy * m_data.gain.first / (recentMax + 1e-5f), 0.f, 1.f);
        const float eased = m_easing.getEasing(norm); //applyEasing(norm, m_data.easingType);

        float x = static_cast<float>(i) * binWidth;
        const float y = size.y - (eased * m_data.heightMod.first);

        auto spawnParticle = [&](float posY)
        {
          auto* particle = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(energy, m_ctx.globalInfo.elapsedTimeSeconds)
          );
          particle->setPosition({ x, posY });
          ParticleLayoutBase::notifyBehaviorOnSpawn(particle);
        };

        spawnParticle(y);

        if (m_data.enableMirroring.first)
          spawnParticle(size.y - y);  // vertical mirror
      }

      m_timedBuffer.stopTimerAndAddSample();
    }

    void drawMenu() override
    {

      if ( ImGui::TreeNode( "Plot Line Layout" ) )
      {
        m_particleGeneratorManager.drawMenu();
        ImGui::Separator();

        m_particleGeneratorManager.getParticleGenerator()->drawMenu();

        ImGui::SeparatorText( "Plot Line Options" );
        EXPAND_SHADER_IMGUI(PLOT_LINE_VISUALIZER_PARAMS, m_data)

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

  private:
    layout::plotline::PlotLineVisualizerData_t m_data;
    RingBufferAverager m_timedBuffer;
    PercentageEasing m_easing;
    MaxEnergyTracker m_recentMax;
  };

}