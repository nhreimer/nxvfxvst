#pragma once

namespace nx
{
  class MultichannelPipeline
  {
  public:

    explicit MultichannelPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    ~MultichannelPipeline()
    {
      for ( auto& pair : m_channels )
      {
        for ( int i = 0; i < pair.second.size(); ++i )
          delete pair.second[ i ];
      }
    }

    void addChannel()
    {
      int16_t channel = 0;
      for ( int i = 0; i < m_channels.size(); ++i )
        ++channel;

      auto key = m_channels.find( channel );
      if ( key == m_channels.end() )
      {
        m_channels.insert( { channel, {} } );
        key = m_channels.find( channel );
      }

      key->second.emplace_back( new ChannelPipeline( m_globalInfo ) );
    }

    void removeChannel( const int16_t channel, const int position )
    {
      delete m_channels.at( channel ).at( position );

      m_channels.at( channel ).erase(
        m_channels.at( channel ).begin() + position );
    }

    void processMidiEvent( const Midi_t& midi )
    {
      const auto key = m_channels.find( midi.channel );
      if ( key != m_channels.end() )
      {
        // send the event to all pipelines in the channel
        for ( int i = 0; i < key->second.size(); ++i )
          key->second[ i ]->processMidiEvent( midi );
      }
    }

    void draw( sf::RenderWindow& window ) const
    {
      for ( auto & pair : m_channels )
      {
        for ( int i = 0; i < pair.second.size(); ++i )
          pair.second[ i ]->draw( window );
      }
    }

    void update( const sf::Time& deltaTime ) const
    {
      for ( auto & pair : m_channels )
      {
        for ( int i = 0; i < pair.second.size(); ++i )
          pair.second[ i ]->update( deltaTime );
      }
    }

    void drawMenu() const
    {
      for ( auto& pair : m_channels )
      {
        ImGui::Begin(
            std::format( "Channel {}",pair.first + 1 ).c_str(),
          nullptr,
          ImGuiWindowFlags_AlwaysAutoResize );
        {
          for ( int i = 0; i < pair.second.size(); ++i )
            pair.second[ i ]->drawMenu();
        }
        ImGui::End();
      }
    }

  private:

    void drawChannelMap()
    {
      ImGui::Begin(
    "Multichannel Pipeline",
  nullptr,
         ImGuiWindowFlags_AlwaysAutoResize );
      {
        for ( auto & pair : m_channels )
        {

          for ( int i = 0; i < pair.second.size(); ++i )
          {
            if ( i > 0 )
            {
              ImGui::SameLine();
              ImGui::Text( "->" );
              ImGui::SameLine();
            }

            if ( ImGui::Checkbox(
              std::format( "C{}:P{}", pair.first, i ).c_str(), &m_isSelected ) )
            {
              if ( m_channelPipelineSelected != i ||
                   m_channelSelected != pair.first )
              {
                // TODO: add swap
              }
            }
          }
        }
      }
      ImGui::End();
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    std::unordered_map< int16_t,
                        std::vector< ChannelPipeline * > > m_channels;

    bool m_isSelected { false };
    int16_t m_channelSelected { 1 };
    int m_channelPipelineSelected { 0 };
  };
}