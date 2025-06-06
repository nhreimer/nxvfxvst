#pragma once

#include "models/EventRecorder.hpp"
#include "models/IEncoder.hpp"

#include "models/encoder/FrameRateLock.hpp"

namespace nx
{
  class RawRGBAEncoder final : public IEncoder
  {

    struct RawEncoderHeader_t
    {
      uint32_t width { 0 };
      uint32_t height { 0 };
      int64_t frameCount { 0 };
      double firstFrameInSeconds { 0.f };

      // the offset between the playhead and the first frame
      // which means if you render the audio down at the same playhead
      // then you account for the offset to match the audio and video by
      double offsetInSeconds { 0.f };
    };

    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v libx264 -pix_fmt yuv420p out.mp4
    // ffmpeg -f rawvideo -pix_fmt rgba -s 800x600 -r 60 -i output.rgba -c:v ffv1 out.mkv
  public:

    explicit RawRGBAEncoder( const EncoderData_t& data );

    ~RawRGBAEncoder() override;

    void addMidiEvent( const Midi_t& midiEvent ) override
    {
      m_recorder.addEvent( m_header.frameCount,
                           midiEvent,
                           m_clock.getElapsedTime().asSeconds() );
    }

    void writeFrame( const double playhead, const sf::RenderWindow& window ) override;

    bool isRecording() const override { return m_isRecording; }

    private:

      const EncoderData_t& m_data;

      std::ofstream m_file;
      std::string m_filename;
      std::string m_metadataFilename;
      sf::Texture m_texture;

      RawEncoderHeader_t m_header;
      bool m_isRecording { false };

      sf::Clock m_clock;
      EventRecorder m_recorder;

      FrameRateLock m_frameLock;
  };
}