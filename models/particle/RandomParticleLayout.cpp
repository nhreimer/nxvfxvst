#include "models/particle/RandomParticleLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json RandomParticleLayout::serialize() const
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    return j;
  }

  void RandomParticleLayout::deserialize(const nlohmann::json &j)
  {
    ParticleHelper::deserialize( m_data, j );
    if (j.contains("behaviors"))
      m_behaviorPipeline.loadPipeline(j.at("behaviors"));
  }

  void RandomParticleLayout::drawMenu()
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Random Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void RandomParticleLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    auto * p = m_particles.emplace_back( new TimedParticle_t() );
    p->shape.setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::initializeParticle( p, midiEvent );
  }

}