#include "models/channel/MidiChannelPipeline.hpp"

namespace nx
{

  MidiChannelPipeline::MidiChannelPipeline( PipelineContext& context, const int32_t channelId )
    : ChannelPipeline( context, channelId )
  {}

  nlohmann::json MidiChannelPipeline::saveChannelPipeline() const
  {
    nlohmann::json j = {};
    j[ "channel" ][ "particles" ] = m_particleLayout.serialize();
    j[ "channel" ][ "modifiers" ] = m_modifierPipeline.saveModifierPipeline();
    j[ "channel" ][ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
    return j;
  }

  void MidiChannelPipeline::loadChannelPipeline( const nlohmann::json& j )
  {
    if ( j.contains( "channel" ) )
    {
      const auto& jchannel = j[ "channel" ];
      if ( jchannel.contains( "particles" ) )
        m_particleLayout.deserialize( jchannel.at( "particles" ) );
      else
      {
        // a channel particle should always be available
        LOG_WARN( "Deserializer: Channel particle layout not found" );
      }

      if ( jchannel.contains( "modifiers" ) )
        m_modifierPipeline.loadModifierPipeline( jchannel.at( "modifiers" ) );
      else
      {
        // optional whether any exist
        LOG_DEBUG( "Deserializer: No channel modifiers found" );
      }

      if ( jchannel.contains( "shaders" ) )
        m_shaderPipeline.loadShaderPipeline( jchannel.at( "shaders" ) );
      else
      {
        // optional whether any exist
        LOG_DEBUG( "Deserializer: No channel shaders found" );
      }
    }
    else
    {
      LOG_WARN( "Deserializer: No channel data found" );
    }
  }

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