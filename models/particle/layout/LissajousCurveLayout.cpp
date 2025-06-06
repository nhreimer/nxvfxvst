#include "models/particle/layout/LissajousCurveLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json LissajousCurveLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "phaseAStep" ] = m_data.phaseAStep;
    j[ "phaseBStep" ] = m_data.phaseBStep;
    j[ "phaseDelta" ] = m_data.phaseDelta;
    j[ "phaseSpread" ] = m_data.phaseSpread;

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void LissajousCurveLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.phaseAStep = j.value( "phaseAStep", 2.0f );
      m_data.phaseBStep = j.value( "phaseBStep", 3.0f );
      m_data.phaseDelta = j.value( "phaseDelta", 0.5f );
      m_data.phaseSpread = j.value( "phaseSpread", 0.5f );
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

  void LissajousCurveLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Lissajous Curve Layout" ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      //ImGui::SliderFloat("Phase", &m_data.phase, 0.f, 1.0f);
      ImGui::SliderFloat("a Phase Step", &m_data.phaseAStep, 0.f, 5.0f);
      ImGui::SliderFloat("b Phase Step", &m_data.phaseBStep, 0.f, 5.0f);
      ImGui::SliderFloat("Phase Delta", &m_data.phaseDelta, 0.f, 2.0f);
      ImGui::SliderFloat("Phase Spread", &m_data.phaseSpread, 0.f, 1.0f);

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Separator();
    }
  }

  void LissajousCurveLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
    const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency

    const float localT = m_ctx.globalInfo.elapsedTimeSeconds;

    const float x = ( static_cast< float >( m_ctx.globalInfo.windowSize.x ) * m_data.phaseSpread ) * sin(a * localT + m_data.phaseDelta);
    const float y = ( static_cast< float >( m_ctx.globalInfo.windowSize.y ) * m_data.phaseSpread ) * sin(b * localT);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( { x, y } );
    ParticleLayoutBase::notifyBehaviorOnSpawn( p );
  }

  // sf::Vector2f LissajousCurveLayout::getNextPosition( const Midi_t & midiEvent ) const
  // {
  //   const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
  //   const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency
  //
  //   // this is also the phase
  //   const float t = m_ctx.globalInfo.elapsedTimeSeconds;
  //
  //   const float x = ( m_ctx.globalInfo.windowSize.x * m_data.phaseSpread ) * sin(a * t + m_data.phaseDelta);
  //   const float y = ( m_ctx.globalInfo.windowSize.y * m_data.phaseSpread ) * sin(b * t);
  //
  //   return { x, y };
  // }


}