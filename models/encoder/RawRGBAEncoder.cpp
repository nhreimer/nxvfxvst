#include "models/encoder/RawRGBAEncoder.hpp"

#include <fstream>

#include "helpers/Definitions.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  RawRGBAEncoder::RawRGBAEncoder( const EncoderData_t& data )
    : m_data( data ),
      m_filename( data.outputFilename.data() )
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

  /////////////////////////////////////////////////////////
  /* PUBLIC */
  RawRGBAEncoder::~RawRGBAEncoder()
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
        << "\tfirstFrameInSeconds: " << m_header.firstFrameInSeconds << "\n"
        << "\toffsetInSeconds: " << m_header.offsetInSeconds << "\n"
        << "}" << std::endl;

      headerFileStream.close();
    }

    m_recorder.saveToFile( m_metadataFilename );
  }

  /////////////////////////////////////////////////////////
  /* PUBLIC */
  void RawRGBAEncoder::writeFrame( const double playhead, const sf::RenderWindow& window )
  {
    // don't do anything until the playhead is at or beyond our starting point
    // get the closest, which ensures we'll be within PLAYHEAD_INTERVAL_IN_SECS / 2 distance

    const auto currentDelta = std::abs( m_data.startAtInSeconds - playhead );
    const auto futureDelta = std::abs( m_data.startAtInSeconds - ( playhead + PLAYHEAD_INTERVAL_IN_SECS ) );

    if ( futureDelta < 0 || futureDelta < currentDelta ) return;

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

    if ( m_header.frameCount == 0 )
    {
      m_header.firstFrameInSeconds = playhead;

      // see how far off we are
      m_header.offsetInSeconds = playhead - m_data.startAtInSeconds;
    }

    ++m_header.frameCount;
  }

}