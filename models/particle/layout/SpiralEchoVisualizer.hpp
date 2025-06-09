#pragma once

#include "models/audio/IFFTResult.hpp"
#include "models/InterfaceTypes.hpp"
#include "models/audio/MaxEnergyTracker.hpp"

namespace nx
{

  struct SpiralLayoutData_t : ParticleLayoutData_t
  {
    bool isActive = true;
    bool mirrorSpiral = false;
    float gain = 8.f;
    float threshold = 0.1f;
    float startRadius = 100.f;
    float radiusMod = 200.f;
    float tightness = 0.2f;
    float rotationRate = 0.005f;
    float rotationOffset = 0.f; // Updated every tick
    float skewRotation = 0.f;

    float mirrorGainFactor = 0.75f;   // Scales the mirror’s energy
    float mirrorRadialOffset = 30.f;  // Push mirror slightly further or closer
    float mirrorSkewRotation = 0.f;
  };

  class SpiralEchoVisualizer final : public ParticleLayoutBase< SpiralLayoutData_t >
  {
  public:

    explicit SpiralEchoVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return {};
    }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_SpiralEchoVisualizer; }

    void processAudioBuffer( const IFFTResult& fftResult ) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto center = m_ctx.globalInfo.windowHalfSize;
      const float spiralStartRadius = m_data.startRadius;
      const float spiralTightness = m_data.tightness; // e.g., 0.25 = tighter spiral
      constexpr float baseAngleStep = NX_TAU / static_cast<float>(FFT_BINS);

      for ( size_t i = 0; i < FFT_BINS; ++i )
      {
        const float energy = fft[ i ];
        if ( energy < m_data.threshold )
          continue;

        const float recentMax = m_recentMax.updateMaxEnergy( energy );

        const float normMag = std::clamp(energy / (recentMax + 1e-5f), 0.f, 1.f);
        const float eased = Easings::easeOutExpo( normMag );

        const float angle = static_cast<float>(i) * ( baseAngleStep + m_data.skewRotation ) + m_data.rotationOffset;
        const float radius = spiralStartRadius + spiralTightness * static_cast< float >(i) + eased * m_data.radiusMod;

        { // clockwise spiral
          const sf::Vector2f pos =
          {
            center.x + std::cos(angle) * radius,
            center.y + std::sin(angle) * radius
          };

          auto* particle = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(energy, m_ctx.globalInfo.elapsedTimeSeconds));

          particle->setPosition(pos);
          ParticleLayoutBase::notifyBehaviorOnSpawn(particle);
        }

        // mirror counter-clockwise spiral
        if ( m_data.mirrorSpiral )
        {
          const float mirrorEnergy = energy * m_data.mirrorGainFactor;
          const float mirrorRadius = radius + m_data.mirrorRadialOffset;
          const float mirroredAngle = -(static_cast<float>(i) * ( baseAngleStep + m_data.mirrorSkewRotation ) + angle);

          const sf::Vector2f mirrorPos =
          {
            center.x + std::cos(mirroredAngle) * mirrorRadius,
            center.y + std::sin(mirroredAngle) * mirrorRadius
          };

          auto* mirror = m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle(mirrorEnergy, m_ctx.globalInfo.elapsedTimeSeconds));
          mirror->setPosition(mirrorPos);
          ParticleLayoutBase::notifyBehaviorOnSpawn(mirror);
        }
      }

      m_data.rotationOffset += m_data.rotationRate;
      if ( m_data.rotationOffset > NX_TAU )
        m_data.rotationOffset -= NX_TAU;

      m_timedBuffer.stopTimerAndAddSample();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Spiral Echo Layout") )
      {
        m_particleGeneratorManager.drawMenu();
        ImGui::Separator();

        m_particleGeneratorManager.getParticleGenerator()->drawMenu();
        ImGui::SeparatorText( "Spiral Echo Options" );

        ImGui::Checkbox("Active", &m_data.isActive);
        ImGui::SliderFloat("Threshold", &m_data.threshold, 0.f, 0.5f);
        ImGui::SliderFloat("Gain", &m_data.gain, 0.1f, 20.f);
        ImGui::SliderFloat("Start Radius", &m_data.startRadius, 0.f, 400.f);
        ImGui::SliderFloat("Radius Mod", &m_data.radiusMod, 0.f, 1000.f);
        ImGui::SliderFloat("Spiral Tightness", &m_data.tightness, 0.01f, 10.f);
        ImGui::SliderFloat("Rotation Rate", &m_data.rotationRate, 0.f, 0.2f);
        ImGui::SliderFloat( "Skew", &m_data.skewRotation, -NX_PI, NX_PI );

        ImGui::Separator();
        ImGui::Checkbox("Mirror", &m_data.mirrorSpiral);
        ImGui::SliderFloat("Mirror Gain", &m_data.mirrorGainFactor, 0.f, 1.f);
        ImGui::SliderFloat( "Radial Offset", &m_data.mirrorRadialOffset, 0.f, 150.f );
        ImGui::SliderFloat( "Mirror Skew", &m_data.mirrorSkewRotation, -NX_PI, NX_PI );

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    SpiralLayoutData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMax;
  };

}