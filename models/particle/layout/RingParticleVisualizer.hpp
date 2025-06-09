#pragma once

#include "models/InterfaceTypes.hpp"
#include "models/audio/MaxEnergyTracker.hpp"
#include "models/data/ParticleData_t.hpp"
#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "utils/RingBufferAverager.hpp"

namespace nx
{

  struct RingParticleLayoutData_t : public ParticleLayoutData_t
  {
    bool isActive = true;
    float gain = 10.f;
    float baseRingRadius = 100.f;
    float radiusMod = 100.f;

    // for when to not display data
    float threshold = 0.1f;
  };

  class RingParticleVisualizer final : public ParticleLayoutBase< RingParticleLayoutData_t >
  {
  public:
    explicit RingParticleVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j =
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };

      j[ "gain" ] = m_data.gain;
      j[ "baseRingRadius" ] = m_data.baseRingRadius;
      j[ "radiusMod" ] = m_data.radiusMod;
      j[ "threshold" ] = m_data.threshold;

      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
      j[ "easings" ] = m_fadeEasing.serialize();

      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.gain = j.value( "gain", 10.f );
        m_data.baseRingRadius = j.value( "baseRingRadius", 100.f );
        m_data.radiusMod = j.value( "radiusMod", 100.f );
        m_data.threshold = j.value( "threshold", 0.1f );
      }

      if ( j.contains( "particleGenerator" ) )
        m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

      if ( j.contains( "easings" ) )
        m_fadeEasing.deserialize( j.at( "easings" ) );
    }

    [[nodiscard]] E_LayoutType getType() const override
    {
      return E_LayoutType::E_RingParticleVisualizer;
    }

    void processAudioBuffer( const IFFTResult& fftResult ) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();

      constexpr float angleStep = NX_TAU / static_cast<float>(FFT_BINS);

      for ( auto i = 0; i < FFT_BINS; ++i )
      {
        if ( fft[ i ] < m_data.threshold )
          continue;

        const float mag = fft[i] * m_data.gain;
        const float recentMax = m_recentMax.updateMaxEnergy(mag);

        const auto normMag = std::clamp(mag / (recentMax + 1e-5f), 0.f, 1.f);
        const auto eased = Easings::easeOutExpo(normMag);

        const float angle = static_cast< float >(i) * angleStep;
        const float radius = m_data.baseRingRadius + eased * m_data.radiusMod;

        const sf::Vector2f pos =
        {
          m_ctx.globalInfo.windowHalfSize.x + std::cos( angle ) * radius,
          m_ctx.globalInfo.windowHalfSize.y + std::sin( angle ) * radius
        };

        auto * particle =
          m_particles.emplace_back(
            m_particleGeneratorManager.getParticleGenerator()->createParticle( mag, m_ctx.globalInfo.elapsedTimeSeconds ) );

        particle->setPosition( pos );

        ParticleLayoutBase::notifyBehaviorOnSpawn( particle );
      }

      m_timedBuffer.stopTimerAndAddSample();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Ring Particle Options" ) )
      {
        m_particleGeneratorManager.drawMenu();
        ImGui::Separator();

        m_particleGeneratorManager.getParticleGenerator()->drawMenu();
        ImGui::SeparatorText( "Ring Particle Options" );

        ImGui::SliderFloat( "Threshold", &m_data.threshold, 0.f, 1.f );
        ImGui::SliderFloat( "Gain", &m_data.gain, 0.f, 20.f );
        ImGui::SliderFloat( "Base Ring Radius", &m_data.baseRingRadius, 0.f, static_cast< float >(m_ctx.globalInfo.windowSize.x) );
        ImGui::SliderFloat( "Radius Mod", &m_data.radiusMod, 0.f, 400.f );

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    RingParticleLayoutData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMax;
  };

}