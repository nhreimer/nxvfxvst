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

#ifdef DEBUG

    static void printAllCodecs()
    {
      const AVCodec * current_codec = nullptr;
      void * i = nullptr;
      while ((current_codec = av_codec_iterate(&i)))
      {
        if (av_codec_is_encoder(current_codec))
        {
          if ( current_codec->type == AVMEDIA_TYPE_VIDEO )
            LOG_INFO( "Encoder: {}", current_codec->name );
        }
      }
    }

#endif

    explicit FFMpegEncoder( const EncoderData_t& data );
    void addMidiEvent( const Midi_t& midiEvent ) override;
    bool isRecording() const override { return m_isRecording; }
    void writeFrame(const sf::RenderWindow& window) override;
    ~FFMpegEncoder() override;

  private:

    bool sendFrame() const;
    const AVCodec* getCodec() const;

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
  };
}