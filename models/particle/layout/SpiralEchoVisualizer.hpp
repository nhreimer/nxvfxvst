#pragma once

#include "models/audio/IFFTResult.hpp"
#include "models/InterfaceTypes.hpp"
#include "models/audio/MaxEnergyTracker.hpp"

namespace nx
{

namespace layout::spirallayout
{
#define SPIRAL_LAYOUT_PARAMS(X)                                                                            \
X(gain,                 float,  8.f,   0.f,   100.f, "Multiplier for input energy",             true)      \
X(threshold,            float,  0.1f,  0.f,   1.f,   "Trigger threshold for particle spawn",    true)      \
X(startRadius,          float,  100.f, 0.f,   2000.f,"Base spiral radius",                      true)      \
X(radiusMod,            float,  200.f, 0.f,   2000.f,"Modulation amount for radius",            true)      \
X(tightness,            float,  0.2f,  0.001f, 10.f, "Tightness of spiral arms",                true)      \
X(rotationRate,         float,  0.005f, 0.f,  1.f,   "Rotation speed of spiral (radians/tick)", true)      \
X(rotationOffset,       float,  0.f,   -360.f, 360.f,"Offset added each frame (auto-updated)",  true)      \
X(skewRotation,         float,  0.f,   -NX_PI, NX_PI,"Additional skew rotation",                true)      \
X(mirrorSpiral,         bool,   false, 0,     1,     "Enable mirrored spiral effect",           false)     \
X(mirrorGainFactor,     float,  0.75f, 0.f,   1.f,   "Gain multiplier for mirror copy",         true)      \
X(mirrorRadialOffset,   float,  30.f,  -500.f, 500.f,"Radial offset for mirror copy",           true)      \
X(mirrorSkewRotation,   float,  0.f,   -NX_PI, NX_PI,"Skew rotation for mirror copy",           true)

  struct SpiralLayoutData_t
  {
    bool isActive = true;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(SPIRAL_LAYOUT_PARAMS)
  };

  enum class E_SpiralLayoutParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(SPIRAL_LAYOUT_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_SpiralLayoutParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(SPIRAL_LAYOUT_PARAMS)
  };

}

  class SpiralEchoVisualizer final
    : public ParticleLayoutBase< layout::spirallayout::SpiralLayoutData_t >
  {
  public:

    explicit SpiralEchoVisualizer( PipelineContext& ctx )
      : ParticleLayoutBase( ctx )
    {
      EXPAND_SHADER_VST_BINDINGS(SPIRAL_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j =
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };

      EXPAND_SHADER_PARAMS_TO_JSON(SPIRAL_LAYOUT_PARAMS)

      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(SPIRAL_LAYOUT_PARAMS)
      }

      if ( j.contains( "particleGenerator" ) )
        m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_SpiralEchoVisualizer; }

    void processAudioBuffer( const IFFTResult& fftResult ) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      const auto center = m_ctx.globalInfo.windowHalfSize;
      const float spiralStartRadius = m_data.startRadius.first;
      const float spiralTightness = m_data.tightness.first; // e.g., 0.25 = tighter spiral
      constexpr float baseAngleStep = NX_TAU / static_cast<float>(FFT_BINS);

      for ( size_t i = 0; i < FFT_BINS; ++i )
      {
        const float energy = fft[ i ];
        if ( energy < m_data.threshold.first )
          continue;

        const float recentMax = m_recentMax.updateMaxEnergy( energy );

        const float normMag = std::clamp(energy / (recentMax + 1e-5f), 0.f, 1.f);
        const float eased = Easings::easeOutExpo( normMag );

        const float angle = static_cast<float>(i) * ( baseAngleStep + m_data.skewRotation.first ) + m_data.rotationOffset.first;
        const float radius = spiralStartRadius + spiralTightness * static_cast< float >(i) + eased * m_data.radiusMod.first;

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
        if ( m_data.mirrorSpiral.first )
        {
          const float mirrorEnergy = energy * m_data.mirrorGainFactor.first;
          const float mirrorRadius = radius + m_data.mirrorRadialOffset.first;
          const float mirroredAngle = -(static_cast<float>(i) * ( baseAngleStep + m_data.mirrorSkewRotation.first ) + angle);

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

      m_data.rotationOffset.first += m_data.rotationRate.first;
      if ( m_data.rotationOffset.first > NX_TAU )
        m_data.rotationOffset.first -= NX_TAU;

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

        EXPAND_SHADER_IMGUI(SPIRAL_LAYOUT_PARAMS, m_data)

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    layout::spirallayout::SpiralLayoutData_t m_data;
    RingBufferAverager m_timedBuffer;
    MaxEnergyTracker m_recentMax;
  };

}