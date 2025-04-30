#pragma once

#include "models/particle/SpiralParticleLayout.hpp"

#include "helpers/MathHelper.hpp"
#include "helpers/MidiHelper.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json SpiralParticleLayout::serialize() const
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    return j;
  }

  void SpiralParticleLayout::deserialize(const nlohmann::json &j)
  {
    ParticleHelper::deserialize( m_data, j );
    if (j.contains("behaviors"))
      m_behaviorPipeline.loadPipeline(j.at("behaviors"));
  }

  void SpiralParticleLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    auto * p = m_particles.emplace_back( new TimedParticle_t() );
    p->shape.setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::initializeParticle( p, midiEvent );
  }

  void SpiralParticleLayout::drawMenu()
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Spiral Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );
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