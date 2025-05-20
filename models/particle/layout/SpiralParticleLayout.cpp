#pragma once

#include "models/particle/layout/SpiralParticleLayout.hpp"

#include "helpers/MathHelper.hpp"
#include "helpers/MidiHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json SpiralParticleLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void SpiralParticleLayout::deserialize(const nlohmann::json &j)
  {
    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );
    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );
    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void SpiralParticleLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );
  }

  void SpiralParticleLayout::drawMenu()
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Spiral Layout " ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();
      getEasing().drawMenu();
      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  sf::Vector2f SpiralParticleLayout::getNextPosition( const Midi_t& midiNote ) const
  {
    const auto noteInfo = MidiHelper::getMidiNote( midiNote.pitch );

    const auto noteNumber = std::get< 0 >( noteInfo );
    const auto noteOctave = std::get< 1 >( noteInfo );

    const auto position = MathHelper::getAnglePosition( 12,
                                                    noteNumber,
                                                    static_cast< float >( noteOctave ),
                                                    static_cast< float >( noteOctave ) );

    return { m_ctx.globalInfo.windowHalfSize.x + position.x,
             m_ctx.globalInfo.windowHalfSize.y + position.y };
  }

;

}