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
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();

    return j;
  }

  void RandomParticleLayout::deserialize(const nlohmann::json &j)
  {
    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
  }

  void RandomParticleLayout::drawMenu()
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Random Layout " ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void RandomParticleLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );
  }

}