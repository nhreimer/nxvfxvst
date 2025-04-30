#include "models/encoder/EncoderFactory.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  FFMpegEncoder::FFMpegEncoder( const EncoderData_t& data )
  {
    auto errCode = avformat_alloc_output_context2(&m_formatCtx, nullptr, nullptr, data.outputFilename.data());
    if ( errCode != 0 )
    {
      LOG_ERROR( "Failed to allocate output context: {}", errCode );
      return;
    }

    auto * fmt = m_formatCtx->oformat;

    const auto * codec = getCodec( data.codecName );
    if ( codec == nullptr )
    {
      LOG_ERROR( "failed to find an appropriate mp4 codec" );
      return;
    }
    m_stream = avformat_new_stream(m_formatCtx, codec);
    m_stream->id = static_cast< int >(m_formatCtx->nb_streams - 1 );

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

    // set the options
    av_opt_set( m_codecCtx->priv_data, "preset", data.mp4PresetOption.c_str(), 0 );
    av_opt_set( m_codecCtx->priv_data, "tune", data.mp4TuningOption.c_str(), 0 );

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
    m_metadataFilename = std::string( data.outputFilename.data() );
    m_metadataFilename.append( ".events.json" );
    m_clock.restart();
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void FFMpegEncoder::addMidiEvent( const Midi_t& midiEvent )
  {
    m_recorder.addEvent( m_frameCount, midiEvent, m_clock.getElapsedTime().asSeconds() );
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void FFMpegEncoder::writeFrame(const sf::RenderWindow& window)
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

  /////////////////////////////////////////////////////////
  /// PUBLIC
  FFMpegEncoder::~FFMpegEncoder()
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

    m_recorder.saveToFile( m_metadataFilename );
    LOG_INFO( "Encoder shutdown successful" );
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC STATIC
  const std::vector< std::string >& FFMpegEncoder::getAvailableCodecs()
  {
    if ( m_availableCodecs.empty() )
    {
      const AVCodec * current_codec = nullptr;
      void * i = nullptr;
      while ( ( current_codec = av_codec_iterate( &i ) ) )
      {
        if ( av_codec_is_encoder( current_codec ) )
        {
          if ( current_codec->type == AVMEDIA_TYPE_VIDEO )
            m_availableCodecs.push_back( std::string( current_codec->name ) );
        }
      }
    }

    return m_availableCodecs;
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////

  bool FFMpegEncoder::sendFrame() const
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

  /////////////////////////////////////////////////////////
  /// PRIVATE
  const AVCodec* FFMpegEncoder::getCodec( const std::string& codecName )
  {
    const auto * codec = avcodec_find_encoder_by_name( codecName.c_str() );

    if ( !codec )
    {
      LOG_ERROR( "failed to find codec: {}", codecName );
    }

    return codec;
  }
}