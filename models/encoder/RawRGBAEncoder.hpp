#pragma once

#include <fstream>

#include "models/EventRecorder.hpp"
#include "models/IEncoder.hpp"

namespace nx
{
  class RawRGBAEncoder final : public IEncoder
  {

    struct RawEncoderHeader_t
    {
      uint32_t width { 0 };
      uint32_t height { 0 };
      int64_t frameCount { 0 };
    };

    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v libx264 -pix_fmt yuv420p out.mp4
    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v ffv1 out.mkv
  public:

    explicit RawRGBAEncoder( const EncoderData_t& data )
      : m_filename( data.outputFilename.data() )
    {
      m_file.open( m_filename, std::ios::binary );
      if ( m_file.fail() )
      {
        LOG_ERROR( "Failed to open file '{}'", m_filename );
      }
      else
      {
        m_isRecording = true;
        LOG_INFO( "Successfully opened file '{}'", m_filename );
      }

      m_metadataFilename = std::string( data.outputFilename.data() );
      m_metadataFilename.append( ".events.json" );
      m_clock.restart();
    }

    ~RawRGBAEncoder() override
    {
      if ( m_file.is_open() )
      {
        m_file.close();
        LOG_INFO( "Closed encoder." );
      }

      std::ofstream headerFileStream;
      std::string headerFileName( m_filename );
      headerFileName.append( ".json" );

      headerFileStream.open( headerFileName );
      if ( !headerFileStream.is_open() )
      {
        LOG_ERROR( "Failed to open file '{}'", headerFileName );
      }
      else
      {
        LOG_INFO( "Successfully opened file '{}'", headerFileName );
        headerFileStream << "{\n"
          << "\twidth: " << m_header.width << ", \n"
          << "\theight: " << m_header.height << ", \n"
          << "\tframeCount: " << m_header.frameCount << "\n"
          << "}" << std::endl;

        headerFileStream.close();
      }

      m_recorder.saveToFile( m_metadataFilename );
    }

    void addMidiEvent( const Midi_t& midiEvent ) override
    {
      m_recorder.addEvent( m_header.frameCount, midiEvent, m_clock.getElapsedTime().asSeconds() );
    }

    void writeFrame( const sf::RenderWindow& window ) override
    {
      if ( m_texture.getSize() != window.getSize() )
      {
        m_header.height = window.getSize().y;
        m_header.width = window.getSize().x;

        if ( !m_texture.resize( window.getSize() ) )
        {
          LOG_ERROR( "Failed to resize encoder texture" );
        }
        else
        {
          LOG_INFO( "Successfully resized encoder texture" );
        }
      }

      m_texture.update( window );
      const auto img = m_texture.copyToImage();
      const auto * pixels = img.getPixelsPtr();
      const auto byteCount = window.getSize().x * window.getSize().y * 4; // RGBA
      m_file.write( reinterpret_cast< const char* >( pixels ), byteCount );
      ++m_header.frameCount;
    }

    bool isRecording() const override { return m_isRecording; }

    private:

      std::ofstream m_file;
      std::string m_filename;
      std::string m_metadataFilename;
      sf::Texture m_texture;

      RawEncoderHeader_t m_header;
      bool m_isRecording { false };

      sf::Clock m_clock;
      EventRecorder m_recorder;
  };
}