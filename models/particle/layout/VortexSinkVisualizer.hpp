#pragma once

#include "models/particle/layout/ParticleLayoutBase.hpp"
#include "helpers/MathHelper.hpp"

namespace nx
{

namespace layout::vortexsink
{
#define VORTEX_SINK_VISUALIZER_PARAMS(X)                                                                 \
X(baseRadius,        float, 300.f,   0.f,   2000.f, "Initial radius for spawn zone",          true)      \
X(spiralSpeed,       float,   5.f,   0.f,   50.f,   "Spinning angular speed (radians/sec)",   true)      \
X(inwardSpeed,       float,  50.f,   0.f,   500.f,  "How fast particles are pulled inward",   true)      \
X(gain,              float, 100.f,   0.f,   500.f,  "Force multiplier (acceleration gain)",   true)      \
X(threshold,         float, 0.02f,   0.f,   1.f,    "Minimum distance before reset triggers", true)      \
X(radiusMod,         float, 250.f,   0.f,   2000.f, "Radius modulation value",                true)      \
X(resetInSeconds,    float,   2.f,   0.01f, 10.f,   "Time before particle resets",            true)      \
X(wobbleFrequency,   float,   4.f,   0.f,   20.f,   "Wobble cycle frequency",                 true)      \
X(wobblePhaseOffset, float, 0.2f,    0.f,   2.f,    "Phase offset for wobble motion",         true)      \
X(wobbleAmplitude,   float, 0.2f,    0.f,   1.f,    "Amplitude of the vortex wobble",         true)

  struct VortexSinkVisualizerData_t : ParticleLayoutData_t
  {
    bool isActive = true;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(VORTEX_SINK_VISUALIZER_PARAMS)
  };

  enum class E_VortexSinkParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(VORTEX_SINK_VISUALIZER_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_VortexSinkParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(VORTEX_SINK_VISUALIZER_PARAMS)
  };
}

class VortexSinkVisualizer final
  : public ParticleLayoutBase< layout::vortexsink::VortexSinkVisualizerData_t >
{
public:

  explicit VortexSinkVisualizer(PipelineContext& ctx)
    : ParticleLayoutBase(ctx)
  {
    EXPAND_SHADER_VST_BINDINGS(VORTEX_SINK_VISUALIZER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  [[nodiscard]]
  nlohmann::json serialize() const override
  {
    nlohmann::json j =
    {
   { "type", SerialHelper::serializeEnum( getType() ) }
    };

    EXPAND_SHADER_PARAMS_TO_JSON(VORTEX_SINK_VISUALIZER_PARAMS)

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_easing.serialize();
    return j;
  }

  void deserialize(const nlohmann::json &j) override
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(VORTEX_SINK_VISUALIZER_PARAMS)
    }

    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

    if ( j.contains( "easings" ) )
      m_easing.deserialize( j.at( "easings" ) );
  }

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

      const auto currentRadius = m_data.baseRadius.first;

      EXPAND_SHADER_IMGUI(VORTEX_SINK_VISUALIZER_PARAMS, m_data)

      if ( currentRadius != m_data.baseRadius.first )
      {
        m_cursorPosition.setPosition( m_ctx.globalInfo.windowHalfSize,
          sf::Vector2f { m_data.baseRadius.first, m_data.baseRadius.first } );
      }

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::Separator();
      ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

      ImGui::TreePop();
      ImGui::Separator();
    }

    if ( !m_cursorPosition.hasExpired() )
      m_cursorPosition.drawPosition();
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
    if ( time >= m_data.resetInSeconds.first ) m_clock.restart();

    for (int32_t i = 0; i < FFT_BINS; ++i)
    {
      if (fft[i] < m_data.threshold.first)
        continue;

      const float mag = fft[i] * m_data.gain.first;
      const float recentMax = m_recentMaxEnergy.updateMaxEnergy( mag );
      const float eased = m_easing.getEasing( recentMax );
      //Easings::easeOutExpo( recentMax );
      //Easings::easeOutExpo(mag / (recentMax + 1e-5f) );
      //squash(mag / (m_recentMax + 1e-5f));

      const float wobble =
        std::sin( m_data.wobbleFrequency.first * m_ctx.globalInfo.elapsedTimeSeconds +
                  static_cast<float>(i) * m_data.wobblePhaseOffset.first ) * m_data.wobbleAmplitude.first;

      // Compute base angle for bin
      float angle = static_cast<float>(i) * angleStep * wobble;
      float radius = m_data.baseRadius.first + eased * m_data.radiusMod.first;

      // Spiral offset
      //const float time = m_ctx.globalInfo.elapsedTimeSeconds;
      angle += time * m_data.spiralSpeed.first;
      radius -= time * m_data.inwardSpeed.first;

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
  TimedCursorPosition m_cursorPosition;
};

} // namespace nx
