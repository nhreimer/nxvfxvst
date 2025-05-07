#include "models/ChannelPipeline.hpp"

namespace nx
{

  ChannelPipeline::ChannelPipeline( PipelineContext& context, const int32_t channelId )
    : m_ctx( context ),
      m_drawPriority( channelId ),    // the ID is the initial priority. it serves no other purpose.
      m_particleLayout( context ),
      m_modifierPipeline( context ),
      m_shaderPipeline( context )
  {
    // check the 0th value to see whether the static values haven't been written yet
    if ( m_drawPriorityNames[ 0 ].empty() )
    {
      for ( int32_t i = 0; i < MAX_CHANNELS; ++i )
        m_drawPriorityNames[ i ] = std::to_string( i + 1 );
    }
  }

  nlohmann::json ChannelPipeline::saveChannelPipeline() const
  {
    nlohmann::json j = {};
    j[ "channel" ][ "particles" ] = m_particleLayout.serialize();
    j[ "channel" ][ "modifiers" ] = m_modifierPipeline.saveModifierPipeline();
    j[ "channel" ][ "shaders" ] = m_shaderPipeline.saveShaderPipeline();
    return j;
  }

  void ChannelPipeline::loadChannelPipeline( const nlohmann::json& j )
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

  void ChannelPipeline::processMidiEvent( const Midi_t& midiEvent ) const
  {
    m_particleLayout.processMidiEvent( midiEvent );

    m_modifierPipeline.processMidiEvent( midiEvent );

    // notify all shaders of an incoming event
    // which can be used for synchronizing effects on midi hits
    m_shaderPipeline.processMidiEvent( midiEvent );
  }

  void ChannelPipeline::update( const sf::Time& deltaTime ) const
  {
    m_particleLayout.update( deltaTime );
    m_modifierPipeline.update( deltaTime );
    m_shaderPipeline.update( deltaTime );
  }

  // const sf::RenderTexture& ChannelPipeline::draw()
  // {
  //   const auto& modifierTexture = m_modifierPipeline.applyModifiers(
  //     m_particleLayout.getParticleOptions(),
  //     m_particleLayout.getParticles() );
  //
  //   return m_shaderPipeline.draw( modifierTexture );
  // }

  void ChannelPipeline::drawMenu()
  {
    m_particleLayout.drawMenu();

    ImGui::Separator();
    m_modifierPipeline.drawMenu();

    ImGui::Separator();
    m_shaderPipeline.drawMenu();

    ImGui::Separator();
    drawChannelPipelineMenu();
    ImGui::Separator();
  }

  void ChannelPipeline::drawChannelPipelineMenu()
  {
    if ( ImGui::TreeNode( "Channel Options" ) )
    {
      ImGui::Checkbox( "Mute", &m_isBypassed );

      ImGui::SeparatorText( "Channel Blend" );
      MenuHelper::drawBlendOptions( m_blendMode );

      if ( ImGui::BeginCombo( "Draw Priority",
                              m_drawPriorityNames[ m_drawPriority ].c_str() ) )
      {
        for ( int32_t i = 0; i < MAX_CHANNELS; ++i )
        {
          if ( ImGui::Selectable( m_drawPriorityNames[ i ].c_str(),
                                  i == m_drawPriority ) )
          {
            m_drawPriority = i;
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}