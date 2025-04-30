#pragma once

#include "data/PipelineContext.hpp"
#include "models/ChannelPipeline.hpp"
#include "models/EventRecorder.hpp"
#include "models/encoder/EncoderFactory.hpp"
#include "shapes/TimedMessage.hpp"

#ifdef BUILD_PLUGIN
#include "vst/version.h"
#else
#include "app/version.hpp"
#endif

namespace nx
{
  class MultichannelPipeline
  {

    // TODO: This is hardcoded for now, but it should be adjustable by the user
    static constexpr int MAX_CHANNELS = 4;

  public:
    explicit MultichannelPipeline( PipelineContext& context )
      : m_ctx( context )
    {
      for ( int i = 0; i < m_channels.size(); ++i )
        m_channels[ i ] = std::make_unique< ChannelPipeline >( context );
    }

    ~MultichannelPipeline() = default;

    [[nodiscard]]
    nlohmann::json saveState() const
    {
      nlohmann::json j = nlohmann::json::array();

      for ( int i = 0; i < m_channels.size(); ++i )
      {
        j[ i ] = m_channels[ i ]->saveChannelPipeline();
        LOG_INFO( j[ i ].dump() );
      }
      return j;
    }

    void restoreState( const nlohmann::json &j ) const
    {
      for ( int i = 0; i < j.size(); ++i )
        m_channels[ i ]->loadChannelPipeline( j.at( i ) );
    }

    void processMidiEvent( const Midi_t &midi ) const
    {
      if ( midi.channel < m_channels.size() )
        m_channels.at( midi.channel )->processMidiEvent( midi );

      if ( m_encoder ) m_encoder->addMidiEvent( midi );
    }

    void draw( sf::RenderWindow &window )
    {
      for ( const auto& channel: m_channels )
        channel->draw( window );

      if ( m_encoder )
      {
        if ( m_encoder->isRecording() ) m_encoder->writeFrame( window );
        else
        {
          m_messageClock.setMessage( "Encoder failed. NOT recording." );
          m_encoder.reset( nullptr );
        }
      }
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

      ImGui::Text( "Framerate: %.2f", ImGui::GetIO().Framerate );
      ImGui::Text( "Window Size: %d, %d", m_ctx.globalInfo.windowSize.x, m_ctx.globalInfo.windowSize.y );
      ImGui::Text( "BPM: %0.2f", m_ctx.globalInfo.bpm );

      int32_t offsetChannel = m_selectedChannel + 1;

      // we do this so that the channel is 1 and not 0. it's just a convenience thing
      // but please note that the channel is 0-based index!
      if ( ImGui::SliderInt( "##ChannelSelector",
        &offsetChannel,
        1,
        static_cast< int32_t >( m_channels.size() ),
        "Channel %d" ) )
      {
        m_selectedChannel = offsetChannel - 1;
      }

      ImGui::Separator();

      m_channels[ m_selectedChannel ]->drawMenu();

      ImGui::Separator();
      if ( ImGui::Button( "export" ) )
      {
        const auto json = saveState();
        ImGui::SetClipboardText( json.dump().c_str() );
        m_messageClock.setMessage( "data copied to clipboard." );
      }
      ImGui::SameLine();
      if ( ImGui::Button( "import" ) )
      {
        const auto json = std::string( ImGui::GetClipboardText() );
        if ( !json.empty() )
        {
          const auto importedData =
            nlohmann::json::parse( json.c_str(), nullptr, false );

          if ( !importedData.is_discarded() )
          {
            restoreState( importedData );
            m_messageClock.setMessage( "data copied from clipboard." );
          }
          else
            m_messageClock.setMessage( "failed to import: invalid json." );
        }
        else
          m_messageClock.setMessage( "failed to import: empty clipboard." );
      }

      if ( ImGui::TreeNode( "Video Encoder" ) )
      {

        if ( ImGui::RadioButton( "RawRGBA", m_encoderType == E_Encoder::E_RawRGBA ) )
        {
          m_encoderType = E_Encoder::E_RawRGBA;
        }
        else if ( ImGui::RadioButton( "MP4", m_encoderType == E_Encoder::E_MP4 ) )
        {
          m_encoderType = E_Encoder::E_MP4;
        }

        if ( m_encoderType != E_Encoder::E_MP4 )
        {
          // TODO: set the radio dials for HW-enabled
        }

        ImGui::InputText( "Filename ",
                          m_encoderData.outputFilename.data(),
                          m_encoderData.outputFilename.size() );

        if ( m_encoder )
        {
          if ( ImGui::Button( "Stop Recording" ) )
          {
            m_encoder.reset( nullptr );
            m_messageClock.setMessage( "RECORDING STOPPED!" );
          }
        }
        else if ( ImGui::Button( "Start Recording" ) )
        {
          m_encoderData.size = m_ctx.globalInfo.windowSize;
          m_encoder.reset( nullptr );
          m_encoder = EncoderFactory::create( m_encoderType, m_encoderData );
          if ( m_encoder && m_encoder->isRecording() )
          {
            m_messageClock.setMessage( "RECORDING STARTED!" );
          }
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_messageClock.hasExpired() )
      {
        ImGui::Separator();
        ImGui::TextUnformatted( m_messageClock.getMessage().data() );
      }

      ImGui::Separator();
      ImGui::Text( "%s", BUILD_NUMBER );
      ImGui::End();
    }

  private:

    PipelineContext m_ctx;

    std::array< std::unique_ptr< ChannelPipeline >, MAX_CHANNELS > m_channels;
    TimedMessage m_messageClock;

    int m_selectedChannel { 0 };

    E_Encoder m_encoderType { E_Encoder::E_RawRGBA };
    EncoderData_t m_encoderData {};

    // used for saving to video
    std::unique_ptr< IEncoder > m_encoder;

  };

} // namespace nx
