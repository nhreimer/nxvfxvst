
#include "models/particle/layout/GoldenSpiralLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json GoldenSpiralLayout::serialize() const
  {
    nlohmann::json j =
    {
          { "type", SerialHelper::serializeEnum( getType() ) }
    };

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
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void GoldenSpiralLayout::deserialize(const nlohmann::json &j)
  {
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

    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void GoldenSpiralLayout::drawMenu()
  {
    ImGui::Text("Particles: %zu", m_particles.size());
    ImGui::Separator();
    if (ImGui::TreeNode("Golden Spiral Layout"))
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

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

  void GoldenSpiralLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    const auto pitchSlices =
      static_cast< int32_t >( midiEvent.pitch / static_cast< float >( m_data.depth ) );

    for ( int32_t i = 1; i <= m_data.depth; ++i )
    {
      auto * p = m_particles.emplace_back(
        m_particleGeneratorManager.getParticleGenerator()->createParticle(
          midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

      const auto pos = getSpiralPosition( i * pitchSlices, m_data.depth );
      p->setPosition( pos );
      ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );
    }
  }

  [[nodiscard]]
  sf::Vector2f GoldenSpiralLayout::getSpiralPosition( const int index,
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

} // namespace nx
