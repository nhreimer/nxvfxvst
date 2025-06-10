#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "helpers/MathHelper.hpp"

namespace nx
{

struct VortexSinkVisualizerData_t : ParticleLayoutData_t
{
  float baseRadius = 300.f;
  float spiralSpeed = 5.f; // radians per second
  float inwardSpeed = 50.f; // pixels per second
  float gain = 100.f;
  float threshold = 0.02f;
  float radiusMod = 250.f;
  float resetInSeconds = 2.f;
};

class VortexSinkVisualizer final : public ParticleLayoutBase< VortexSinkVisualizerData_t >
{
public:

  explicit VortexSinkVisualizer(PipelineContext& ctx)
    : ParticleLayoutBase(ctx)
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override { return {}; }

  void deserialize(const nlohmann::json &j) override {}

  [[nodiscard]]
  E_LayoutType getType() const override { return E_LayoutType::E_VortexSinkVisualizer; }

  void drawMenu() override
  {
    if (ImGui::TreeNode("Vortex Sink Layout"))
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();

      m_particleGeneratorManager.getParticleGenerator()->drawMenu();
      ImGui::SeparatorText( "Vortex Sink Options" );

      ImGui::SliderFloat("Base Radius", &m_data.baseRadius, 0.f, 800.f);
      ImGui::SliderFloat("Spiral Speed", &m_data.spiralSpeed, 0.f, 20.f);
      ImGui::SliderFloat("Inward Speed", &m_data.inwardSpeed, 0.f, 500.f);
      ImGui::SliderFloat("Gain", &m_data.gain, 1.f, 1000.f);
      ImGui::SliderFloat("Threshold", &m_data.threshold, 0.f, 1.f);
      ImGui::SliderFloat("Radius Mod", &m_data.radiusMod, 0.f, 500.f);
      ImGui::SliderFloat( "Reset in seconds", &m_data.resetInSeconds, 0.f, 20.f );

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::Separator();
      ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

      ImGui::TreePop();
      ImGui::Separator();
    }
  }

  void processAudioBuffer(const IFFTResult& fftResult) override
  {
    m_timedBuffer.startTimer();

    const auto& fft = fftResult.getSmoothedBuffer();
    const sf::Vector2f center = m_ctx.globalInfo.windowHalfSize;
    constexpr float angleStep = NX_TAU / static_cast<float>(FFT_BINS);

    //m_recentMaxEnergy.resetMaxEnergy();
    //const auto time = m_clock.restart();
    const float time = m_clock.getElapsedTime().asSeconds();
    if ( time >= m_data.resetInSeconds ) m_clock.restart();

    for (int32_t i = 0; i < FFT_BINS; ++i)
    {
      if (fft[i] < m_data.threshold)
        continue;

      const float mag = fft[i] * m_data.gain;
      const float recentMax = m_recentMaxEnergy.updateMaxEnergy( mag );
      const float eased = m_easing.getEasing( recentMax );
      //Easings::easeOutExpo( recentMax );
      //Easings::easeOutExpo(mag / (recentMax + 1e-5f) );
      //squash(mag / (m_recentMax + 1e-5f));

      // Compute base angle for bin
      float angle = static_cast<float>(i) * angleStep;
      float radius = m_data.baseRadius + eased * m_data.radiusMod;

      // Spiral offset
      //const float time = m_ctx.globalInfo.elapsedTimeSeconds;
      angle += time * m_data.spiralSpeed;
      radius -= time * m_data.inwardSpeed;

      // Prevent from collapsing to center
      radius = std::max(radius, 10.f);

      const sf::Vector2f pos =
      {
        center.x + std::cos(angle) * radius,
        center.y + std::sin(angle) * radius
      };

      auto* p = m_particles.emplace_back(
        m_particleGeneratorManager.getParticleGenerator()->createParticle(mag, m_ctx.globalInfo.elapsedTimeSeconds));
      p->setPosition(pos);

      ParticleLayoutBase::notifyBehaviorOnSpawn(p);
    }

    m_timedBuffer.stopTimerAndAddSample();
  }

private:

  RingBufferAverager m_timedBuffer;
  MaxEnergyTracker m_recentMaxEnergy;
  PercentageEasing m_easing;
  sf::Clock m_clock;
};

} // namespace nx
