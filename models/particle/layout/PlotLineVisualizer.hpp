#pragma once

namespace nx
{
  struct PlotLineVisualizerData_t : public ParticleLayoutData_t
  {
    float gain = 0.0f;
    float threshold = 0.1f;
    float heightMod = 200.0f;
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

    void processAudioBuffer( const IFFTResult& fftResult ) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto size = m_ctx.globalInfo.windowSize;
      const float binWidth = static_cast<float>(size.x) / static_cast<float>(FFT_BINS);

      for ( size_t i = 0; i < FFT_BINS; ++i )
      {
        const float mag = fft[i];
        if ( mag < m_data.threshold )
          continue;

        updateMaxEnergy( mag );

        const float eased = squash( std::clamp( mag * m_data.gain / (m_recentMax + 1e-5f), 0.f, 1.f ) );

        const float x = static_cast<float>(i) * binWidth;
        const float y = size.y - (eased * m_data.heightMod);  // bottom-up

        auto * particle = m_particles.emplace_back(
          m_particleGeneratorManager.getParticleGenerator()->createParticle( mag, m_ctx.globalInfo.elapsedTimeSeconds )
        );

        particle->setPosition( { x, y } );

        ParticleLayoutBase::notifyBehaviorOnSpawn( particle );
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

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    static float squash(const float x)
    {
      // Easing: easeOutExpo
      return 1.0f - std::pow(2.0f, -10.0f * x);
    }

    void updateMaxEnergy(const float mag)
    {
      // Smooth decay and track new spikes
      m_recentMax = std::lerp(m_recentMax, mag, 0.05f);
      if (mag > m_recentMax)
        m_recentMax = mag;
    }

  private:
    PlotLineVisualizerData_t m_data;
    RingBufferAverager m_timedBuffer;
    float m_recentMax = 0.0f;
  };

}