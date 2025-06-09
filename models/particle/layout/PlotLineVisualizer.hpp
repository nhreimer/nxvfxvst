#pragma once

namespace nx
{
  struct PlotLineVisualizerData_t : public ParticleLayoutData_t
  {
    float gain = 0.0f;
    float threshold = 0.1f;
    float heightMod = 200.0f;

    bool enableHarmonics = true;
    int harmonicCount = 3;
    float harmonicFalloff = 0.5f;

    bool enableMirroring = true;
    E_EasingType easingType = E_EasingType::E_Sine;
  };

  class PlotLineVisualizer final : public ParticleLayoutBase< PlotLineVisualizerData_t >
  {
  public:

    explicit PlotLineVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }
    void deserialize(const nlohmann::json &j) override {}

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
        if (m_data.enableHarmonics)
        {
          for (int h = 2; h <= m_data.harmonicCount; ++h)
          {
            const size_t harmonicIndex = i * h;
            if (harmonicIndex < FFT_BINS)
              energy += fft[harmonicIndex] * m_data.harmonicFalloff;
            else
              break;
          }
        }

        if (energy < m_data.threshold)
          continue;

        const float recentMax = m_recentMax.updateMaxEnergy( energy );

        // Normalize, apply gain and easing
        const float norm = std::clamp(energy * m_data.gain / (recentMax + 1e-5f), 0.f, 1.f);
        const float eased = m_easing.getEasing(norm); //applyEasing(norm, m_data.easingType);

        float x = static_cast<float>(i) * binWidth;
        const float y = size.y - (eased * m_data.heightMod);

        auto spawnParticle = [&](float posY)
        {
          auto* particle = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(energy, m_ctx.globalInfo.elapsedTimeSeconds)
          );
          particle->setPosition({ x, posY });
          ParticleLayoutBase::notifyBehaviorOnSpawn(particle);
        };

        spawnParticle(y);

        if (m_data.enableMirroring)
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

        ImGui::SliderFloat( "Gain", &m_data.gain, 0.1f, 10.f );
        ImGui::SliderFloat( "Height Mod", &m_data.heightMod, 10.f, m_ctx.globalInfo.windowSize.y );
        ImGui::SliderFloat( "Threshold", &m_data.threshold, 0.f, 1.f );

        ImGui::Checkbox("Enable Harmonics", &m_data.enableHarmonics);
        if (m_data.enableHarmonics)
        {
          ImGui::SliderInt("Harmonic Count", &m_data.harmonicCount, 1, 6);
          ImGui::SliderFloat("Harmonic Falloff", &m_data.harmonicFalloff, 0.f, 1.f);
        }
        ImGui::Checkbox("Vertical Mirroring", &m_data.enableMirroring);

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
    PlotLineVisualizerData_t m_data;
    RingBufferAverager m_timedBuffer;
    PercentageEasing m_easing;
    MaxEnergyTracker m_recentMax;
  };

}