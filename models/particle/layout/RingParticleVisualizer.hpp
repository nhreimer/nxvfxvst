#pragma once

#include "models/audio/IFFTResult.hpp"
#include "models/InterfaceTypes.hpp"
#include "models/audio/MaxEnergyTracker.hpp"
#include "models/data/ParticleData_t.hpp"
#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "utils/RingBufferAverager.hpp"

namespace nx
{

namespace layout::ringparticle
{
#define RING_PARTICLE_LAYOUT_PARAMS(X)                                                                 \
X(gain,            float, 10.f,   0.f,   100.f, "Multiplier for radius response",        true)         \
X(baseRingRadius,  float, 100.f,  0.f,   1000.f, "Starting ring radius",                 true)         \
X(radiusMod,       float, 100.f,  0.f,   1000.f, "Amount added to radius based on gain", true)         \
X(threshold,       float, 0.1f,   0.f,   1.f,    "Minimum level required to trigger",    true)

  struct RingParticleLayoutData_t
  {
    bool isActive = true;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(RING_PARTICLE_LAYOUT_PARAMS)
  };

  enum class E_RingParticleParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(RING_PARTICLE_LAYOUT_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_RingParticleParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(RING_PARTICLE_LAYOUT_PARAMS)
  };
}

  class RingParticleVisualizer final : public ParticleLayoutBase< layout::ringparticle::RingParticleLayoutData_t >
  {
  public:
    explicit RingParticleVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {
      EXPAND_SHADER_VST_BINDINGS(RING_PARTICLE_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j =
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };

      EXPAND_SHADER_PARAMS_TO_JSON(RING_PARTICLE_LAYOUT_PARAMS)

      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(RING_PARTICLE_LAYOUT_PARAMS)
      }

      if ( j.contains( "particleGenerator" ) )
        m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
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
        if ( fft[ i ] < m_data.threshold.first )
          continue;

        const float mag = fft[i] * m_data.gain.first;
        const float recentMax = m_recentMax.updateMaxEnergy(mag);

        const auto normMag = std::clamp(mag / (recentMax + 1e-5f), 0.f, 1.f);
        const auto eased = Easings::easeOutExpo(normMag);

        const float angle = static_cast< float >(i) * angleStep;
        const float radius = m_data.baseRingRadius.first + eased * m_data.radiusMod.first;

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

        const auto currentRadius = m_data.baseRingRadius.first + m_data.radiusMod.first;

        EXPAND_SHADER_IMGUI(RING_PARTICLE_LAYOUT_PARAMS, m_data)

        if ( currentRadius != m_data.baseRingRadius.first + m_data.radiusMod.first )
        {
          const auto totalRadius = m_data.baseRingRadius.first + m_data.radiusMod.first;

          m_timedCursor.setPosition( m_ctx.globalInfo.windowHalfSize,
            sf::Vector2f { totalRadius, totalRadius } );
        }

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

  private:

    layout::ringparticle::RingParticleLayoutData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMax;
    TimedCursorPosition m_timedCursor;
  };

}