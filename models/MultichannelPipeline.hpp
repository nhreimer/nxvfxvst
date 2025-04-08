#pragma once

#include "models/ChannelPipeline.hpp"

#ifdef BUILD_PLUGIN
#include "vst/version.h"
#else
#include "app/version.hpp"
#endif

namespace nx
{
  class MultichannelPipeline
  {

  public:
    explicit MultichannelPipeline( const GlobalInfo_t &globalInfo )
      : m_globalInfo( globalInfo )
    {
      for ( int i = 0; i < m_channels.size(); ++i )
        m_channels[ i ] = std::make_unique< ChannelPipeline >( globalInfo );
    }

    ~MultichannelPipeline() = default;

    nlohmann::json saveState() const
    {
      nlohmann::json j = nlohmann::json::array();

      for ( int i = 0; i < m_channels.size(); ++i )
        j.push_back( m_channels[ i ]->saveChannelPipeline() );

      return j;
    }

    void restoreState( nlohmann::json &j )
    {
      for ( int i = 0; i < j.size(); ++i )
        m_channels[ i ]->loadChannelPipeline( j.at( i ) );
    }

    void processMidiEvent( const Midi_t &midi ) const
    {
      if ( midi.channel < m_channels.size() )
        m_channels.at( midi.channel )->processMidiEvent( midi );
    }

    void draw( sf::RenderWindow &window ) const
    {
      for ( const auto& channel: m_channels )
        channel->draw( window );
    }

    void update( const sf::Time &deltaTime ) const
    {
      for ( const auto& channel: m_channels )
        channel->update( deltaTime );
    }

    void drawMenu()
    {
      ImGui::Begin(
        FULL_PRODUCT_NAME,
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize );

      ImGui::SliderInt( "##ChannelSelector",
        &m_selectedChannel,
        0,
        static_cast< int32_t >( m_channels.size() ) - 1,
        "Channel %d" );

      ImGui::Separator();

      m_channels[ m_selectedChannel ]->drawMenu();

      ImGui::Separator();
      ImGui::Text( "%s", BUILD_NUMBER );
      ImGui::End();
    }

  private:
    const GlobalInfo_t &m_globalInfo;

    // TODO: more than 2 throws an error with ImGui somewhere
    std::array< std::unique_ptr< ChannelPipeline >, 4 > m_channels;

    int m_selectedChannel = 0;
  };
} // namespace nx
