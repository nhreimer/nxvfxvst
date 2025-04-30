#pragma once

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
  #include <libavutil/opt.h>
}

#include "models/IEncoder.hpp"

namespace nx
{
  class FFMpegEncoder final : public IEncoder
  {
  public:

    explicit FFMpegEncoder( const EncoderData_t& data );
    ~FFMpegEncoder() override;

    void addMidiEvent( const Midi_t& midiEvent ) override;
    bool isRecording() const override { return m_isRecording; }
    void writeFrame(const sf::RenderWindow& window) override;

    static const std::vector< std::string >& getAvailableCodecs();

  private:

    bool sendFrame() const;
    static const AVCodec* getCodec( const std::string& codecName );

  private:

    int64_t m_frameCount = 0;

    AVFormatContext* m_formatCtx = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    AVStream* m_stream = nullptr;
    AVFrame* m_frame = nullptr;
    AVPacket* m_pkt = nullptr;
    SwsContext* m_swsCtx = nullptr;
    sf::Texture m_texture;

    std::string m_metadataFilename;

    bool m_isRecording = false;

    sf::Clock m_clock;
    EventRecorder m_recorder;

    inline static std::vector< std::string > m_availableCodecs;
  };
}