#pragma once

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
  #include <ffnvcodec/dynlink_cuda.h>
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

    explicit FFMpegEncoder( const EncoderData_t& data )
    {
      auto errCode = avformat_alloc_output_context2(&m_formatCtx, nullptr, nullptr, data.outputFilename.data());
      if ( errCode != 0 )
      {
        LOG_ERROR( "Failed to allocate output context: {}", errCode );
        return;
      }

      auto * fmt = m_formatCtx->oformat;

      const auto * codec = getCodec();
      if ( codec == nullptr )
      {
        LOG_ERROR( "failed to find an appropriate mp4 codec" );
        return;
      }
      m_stream = avformat_new_stream(m_formatCtx, codec);
      m_stream->id = m_formatCtx->nb_streams - 1;

      m_codecCtx = avcodec_alloc_context3(codec);

      if ( m_codecCtx == nullptr )
      {
        LOG_ERROR( "avcodec_alloc_context3 failed with specified codec" );
        return;
      }

      m_stream->time_base = m_codecCtx->time_base;
      m_codecCtx->codec_id = codec->id; //AV_CODEC_ID_H264;
      m_codecCtx->bit_rate = 800000;
      m_codecCtx->width = data.size.x;
      m_codecCtx->height = data.size.y;
      m_codecCtx->time_base = {1, data.fps};
      m_codecCtx->framerate = {data.fps, 1};
      m_codecCtx->gop_size = 12;
      m_codecCtx->max_b_frames = 2;
      m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

      // av_opt_set(m_codecCtx->priv_data, "preset", "p4", 0); // p1 to p7 (fast to slow)
      // av_opt_set(m_codecCtx->priv_data, "tune", "hq", 0);   // latency / hq / ull

      if (fmt->flags & AVFMT_GLOBALHEADER)
        m_codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

      avcodec_open2(m_codecCtx, codec, nullptr);
      avcodec_parameters_from_context(m_stream->codecpar, m_codecCtx);

      errCode = avio_open(&m_formatCtx->pb, data.outputFilename.data(), AVIO_FLAG_WRITE);
      if ( errCode != 0 )
      {
        LOG_ERROR( "Failed to open output file: {}", errCode );
        return;
      }

      avformat_write_header(m_formatCtx, nullptr);

      m_frame = av_frame_alloc();

      if ( m_frame == nullptr )
      {
        LOG_ERROR( "Failed to allocate output frame" );
        return;
      }

      m_frame->format = m_codecCtx->pix_fmt;
      m_frame->width  = m_codecCtx->width;
      m_frame->height = m_codecCtx->height;
      av_frame_get_buffer(m_frame, 32);

      m_pkt = av_packet_alloc();

      m_swsCtx = sws_getContext(data.size.x, data.size.y, AV_PIX_FMT_RGBA,
                                data.size.x, data.size.y, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);

      m_isRecording = true;
    }

    bool isRecording() const override { return m_isRecording; }

    void writeFrame(const sf::RenderWindow& window) override
    {
      if ( m_texture.getSize() != window.getSize() )
      {
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
      const auto image = m_texture.copyToImage();
      const uint8_t* srcSlice = image.getPixelsPtr();

      // Wrap the RGBA input as a fake AVFrame
      uint8_t* inData[1] = { const_cast<uint8_t*>(srcSlice) };
      const int inLineSize[1] = { 4 * static_cast< int >(window.getSize().x) };

      sws_scale(
        m_swsCtx,
        inData,
        inLineSize,
        0,
        window.getSize().y,
        m_frame->data,
        m_frame->linesize);

      m_frame->pts = m_frameCount++; // 1/frame_rate increments

      m_isRecording = sendFrame();
    }

    ~FFMpegEncoder() override
    {
      if ( m_codecCtx == nullptr )
      {
        LOG_DEBUG( "Codec context not initialized. skipping shutdown" );
        return;
      }

      if ( !sendFrame() )
      {
        LOG_ERROR( "Failed to send final frames" );
      }

      av_write_trailer(m_formatCtx);
      avcodec_free_context(&m_codecCtx);
      av_frame_free(&m_frame);
      av_packet_free(&m_pkt);
      sws_freeContext(m_swsCtx);
      avio_closep(&m_formatCtx->pb);
      avformat_free_context(m_formatCtx);

      LOG_INFO( "Encoder shutdown successful" );
    }

  private:

    bool sendFrame() const
    {
      // Flush encoder
      const auto retCode = avcodec_send_frame(m_codecCtx, m_frame);
      if ( retCode < 0 )
      {
        LOG_ERROR( "Failed to send frame to encoder: {}", retCode );
        return false;
      }

      while (avcodec_receive_packet(m_codecCtx, m_pkt) == 0)
      {
        av_packet_rescale_ts(m_pkt, m_codecCtx->time_base, m_stream->time_base);
        m_pkt->stream_index = m_stream->index;
        av_interleaved_write_frame(m_formatCtx, m_pkt);
        av_packet_unref(m_pkt);
      }

      return true;
    }

    const AVCodec* getCodec() const
    {
      const auto * codec = avcodec_find_encoder_by_name("h264_nvenc");

      if ( !codec )
      {
        LOG_ERROR( "failed to find h264_nvenc codec" );
        //codec = avcodec_find_encoder_by_name("h264_mf");
        // if on windows then it'll default to h264_mf most likely and you'll get
        // a warning. that warning occurs even if you use the "264_mf" name
        codec = avcodec_find_encoder( AV_CODEC_ID_H264 );
      }

      if ( !codec )
      {
        LOG_ERROR( "failed to find h264 codec" );
        codec = avcodec_find_encoder_by_name("mpeg4");
      }

      if ( !codec )
      {
        LOG_ERROR( "failed to find mpeg4 codec" );
      }
      else
      {
        LOG_DEBUG( "Using codec `{}`", codec->long_name );
      }

      return codec;
    }

  private:

    int64_t m_frameCount = 0;

    AVFormatContext* m_formatCtx = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    AVStream* m_stream = nullptr;
    AVFrame* m_frame = nullptr;
    AVPacket* m_pkt = nullptr;
    SwsContext* m_swsCtx = nullptr;
    sf::Texture m_texture;

    bool m_isRecording = false;
  };
}