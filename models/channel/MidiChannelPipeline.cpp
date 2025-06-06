#include "models/channel/MidiChannelPipeline.hpp"

namespace nx
{

  MidiChannelPipeline::MidiChannelPipeline( PipelineContext& context, const int32_t channelId )
    : ChannelPipeline( context, channelId )
  {}

  void MidiChannelPipeline::processMidiEvent( const Midi_t& midiEvent ) const
  {
    m_particleLayout.processMidiEvent( midiEvent );

    m_modifierPipeline.processMidiEvent( midiEvent );

    // notify all shaders of an incoming event
    // which can be used for synchronizing effects on midi hits
    m_shaderPipeline.processMidiEvent( midiEvent );
  }

  void MidiChannelPipeline::update( const sf::Time& deltaTime ) const
  {
    m_particleLayout.update( deltaTime );
    m_modifierPipeline.update( deltaTime );
    m_shaderPipeline.update( deltaTime );
  }

  void MidiChannelPipeline::drawMenu()
  {
    ImGui::Text( "Particle count: %d", m_particleLayout.getParticles().size() );
    ImGui::Separator();
    m_particleLayout.drawMidiMenu();

    ImGui::Separator();
    m_modifierPipeline.drawMenu();

    ImGui::Separator();
    m_shaderPipeline.drawMenu();

    ImGui::Separator();
    drawChannelPipelineMenu();
    ImGui::Separator();
  }
}