#pragma once

#include "models/particle/ParticleLayoutBase.hpp"

namespace nx
{

  struct GoldenSpiralLayoutData_t : public ParticleLayoutData_t
  {
    int depth = 3; // Total number of particles
    float scaleFactor = 1.1f; // How much each radius increases
    float angleOffset = 0.f; // Rotate the whole spiral
    float baseRadius = 3.f; // radius of circle

    float spiralTightness = 1.f;
    bool useClamp = false;
    bool spiralInward = false;

    // the following two are disabled
    float radiusFalloff = 0.98f;
    bool useRadiusFalloff = false;
  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class GoldenSpiralLayout final : public ParticleLayoutBase< GoldenSpiralLayoutData_t >
  {
  public:
    explicit GoldenSpiralLayout(PipelineContext& context)
        : ParticleLayoutBase(context)
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      auto j = ParticleHelper::serialize(m_data, SerialHelper::serializeEnum(getType()));
      j[ "depth" ] = m_data.depth;
      j[ "scaleFactor" ] = m_data.scaleFactor;
      j[ "angleOffset" ] = m_data.angleOffset;
      j[ "baseRadius" ] = m_data.baseRadius;
      j[ "spiralTightness" ] = m_data.spiralTightness;
      j[ "useClamp" ] = m_data.useClamp;
      j[ "useRadiusFalloff" ] = m_data.useRadiusFalloff;
      j[ "radiusFalloff" ] = m_data.radiusFalloff;
      j[ "spiralInward" ] = m_data.spiralInward;
      j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      ParticleHelper::deserialize(m_data, j);

      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.depth = j["depth"].get<int>();
        m_data.scaleFactor = j["scaleFactor"].get<float>();
        m_data.angleOffset = j["angleOffset"].get<float>();
        m_data.baseRadius = j["baseRadius"].get<float>();

        m_data.radiusFalloff = j["radiusFalloff"].get<float>();
        m_data.useClamp = j["useClamp"].get<bool>();
        m_data.spiralTightness = j["spiralTightness"].get<float>();
        m_data.spiralInward = j["spiralInward"].get<bool>();
        m_data.useRadiusFalloff = j["useRadiusFalloff"].get<bool>();
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }

      if (j.contains("behaviors"))
        m_behaviorPipeline.loadPipeline(j.at("behaviors"));
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_GoldenSpiralLayout; }

    void drawMenu() override
    {
      ImGui::Text("Particles: %zu", m_particles.size());
      ImGui::Separator();
      if (ImGui::TreeNode("Golden Spiral Layout"))
      {
        ParticleHelper::drawMenu(*reinterpret_cast< ParticleLayoutData_t * >(&m_data));

        ImGui::SeparatorText( "Spiral Options" );
        ImGui::SliderInt("Particle Count", &m_data.depth, 1, 100);
        ImGui::SliderFloat("Base Radius", &m_data.baseRadius, 0.01f, 10.f);
        ImGui::SliderFloat("Scale Factor", &m_data.scaleFactor, 1.01f, 2.0f);
        ImGui::SliderFloat("Angle Offset", &m_data.angleOffset, 0.f, 360.f);

        ImGui::SeparatorText("Spiral Tweaks");
        ImGui::SliderFloat("Tightness", &m_data.spiralTightness, 0.1f, 2.0f);
        ImGui::Checkbox("Use Clamp", &m_data.useClamp);
        ImGui::Checkbox("Spiral Inward", &m_data.spiralInward);
        //ImGui::Checkbox("Shrink Outer Particles", &m_data.useRadiusFalloff);
        //ImGui::SliderFloat("Radius Falloff", &m_data.radiusFalloff, 0.01f, 2.0f);

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void addMidiEvent(const Midi_t &midiEvent) override
    {
      const auto pitchSlices =
        static_cast< int32_t >( midiEvent.pitch / static_cast< float >( m_data.depth ) );

      for ( int32_t i = 1; i <= m_data.depth; ++i )
      {
        auto * p = m_particles.emplace_back( new TimedParticle_t() );
        const auto pos = getSpiralPosition( i * pitchSlices, m_data.depth );
        p->shape.setPosition( pos );
        ParticleLayoutBase::initializeParticle( p, midiEvent );
      }
    }

  private:

    [[nodiscard]]
    sf::Vector2f getSpiralPosition( const int index,
                                    const int total ) const
    {
      const int i = m_data.spiralInward ? (total - 1) - index : index;

      const float angleDeg = static_cast< float >( i ) * GOLDEN_ANGLE_DEG * m_data.spiralTightness + m_data.angleOffset;
      const float angleRad = angleDeg * NX_D2R; //(3.14159265f / 180.f);

      float radius = 0.f;
      if ( m_data.useClamp )
      {
        radius = m_data.baseRadius * std::pow(m_data.scaleFactor, index);
        const float maxR = m_ctx.globalInfo.windowSize.x * 0.45f;

        if ( radius > maxR )
        {
          const float t = (radius - maxR) / maxR;
          radius = maxR + std::sin( t * NX_PI ) * 20.f; // optional ripple-style squish
        }
      }
      else
        radius = m_data.baseRadius * std::pow(m_data.scaleFactor, i);

      return
      {
        m_ctx.globalInfo.windowHalfSize.x + std::cos( angleRad ) * radius,
        m_ctx.globalInfo.windowHalfSize.y + std::sin( angleRad ) * radius
      };
    }

  private:
    static constexpr float GOLDEN_RATIO = 1.61803398875f;
    static constexpr float GOLDEN_ANGLE_DEG = 137.5f;
  };

} // namespace nx
