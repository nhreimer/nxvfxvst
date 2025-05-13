#include "models/particle/layout/RandomParticleLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json RandomParticleLayout::serialize() const
  {
    nlohmann::json j =
    {
          { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGenerator->serialize();

    return j;
  }

  void RandomParticleLayout::deserialize(const nlohmann::json &j)
  {
    if ( j.contains( "particleGenerator" ) )
      m_particleGenerator->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
  }

  void RandomParticleLayout::drawMenu()
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Random Layout " ) )
    {
      m_particleGenerator->drawMenu();

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void RandomParticleLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    auto * p = m_particles.emplace_back(
      m_particleGenerator->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );
  }

}