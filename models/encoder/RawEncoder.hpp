#pragma once

#include <fstream>

namespace nx
{

  struct IEncoder
  {
    virtual ~IEncoder() = default;
    virtual void writeFrame( const sf::RenderWindow& texture ) = 0;
  };

  class RawEncoder final : public IEncoder
  {

    struct RawEncoderHeader_t
    {
      uint32_t width { 0 };
      uint32_t height { 0 };
      uint32_t frameCount { 0 };
    };


    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v libx264 -pix_fmt yuv420p out.mp4
    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v ffv1 out.mkv
  public:

    explicit RawEncoder( const std::string& filename )
      : m_filename( filename )
    {
      m_file.open( filename, std::ios::binary );
      if ( m_file.fail() )
      {
        LOG_ERROR( "Failed to open file '{}'", filename );
      }
      else
      {
        LOG_INFO( "Successfully opened file '{}'", filename );
      }
    }

    ~RawEncoder() override
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

    private:

      std::ofstream m_file;
      std::string m_filename;
      sf::Texture m_texture;

      RawEncoderHeader_t m_header;
  };

}