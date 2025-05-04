#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include "models/IEncoder.hpp"

/// Not in use at the moment for two reasons:
/// 1. adds 20mb or so to the VST
/// 2. difficulty in getting it to work consistently
namespace nx
{

  namespace priv
  {
    template< typename T >
    struct AVTraits;

    template <>
    struct AVTraits<AVCodecContext>
    {
      static void destroy(AVCodecContext*& ptr)
      {
        ::avcodec_free_context(&ptr);
      }
    };

    template<>
    struct AVTraits< ::AVFormatContext >
    {
      static void destroy( AVFormatContext* ptr )
      {
        ::avformat_free_context( ptr );
      }
    };

    template<>
    struct AVTraits< ::AVFrame >
    {
      static void destroy( AVFrame*& ptr )
      {
        ::av_frame_free( &ptr );
      }
    };

    template<>
    struct AVTraits< ::AVPacket >
    {
      static void destroy( AVPacket *& ptr )
      {
        ::av_packet_free( &ptr );
      }
    };

    template <>
    struct AVTraits<SwsContext>
    {
      static void destroy(SwsContext*& ptr)
      {
        ::sws_freeContext(ptr);
      }
    };

    template<>
    struct AVTraits< ::AVIOContext >
    {
      static void destroy( AVIOContext *& ptr )
      {
        ::avio_closep( &ptr );
      }
    };

    template< typename T >
    struct AVScoped
    {
      T *ptr = nullptr;

      AVScoped() = default;
      explicit AVScoped(T *p) : ptr(p) {}

      ~AVScoped()
      {
        reset();
      }

      AVScoped& operator=(T * p)
      {
        reset( p );
        return *this;
      }

      bool operator==(T * p) const
      {
        return ptr == p;
      }

      T * get() const { return ptr; }
      T ** operator&() { return &ptr; }
      T * operator->() const { return ptr; }

      void reset(T* newPtr = nullptr)
      {
        if (ptr)
          AVTraits<T>::destroy(ptr); // this is now uniform for both T* and T**
        ptr = newPtr;
      }
    };
  }

  class FFMpegEncoder final : public IEncoder
  {


  public:
    explicit FFMpegEncoder(const EncoderData_t &data);
    ~FFMpegEncoder() override;

    void addMidiEvent(const Midi_t &midiEvent) override;
    bool isRecording() const override { return m_isRecording; }
    void writeFrame(const sf::RenderWindow &window) override;

    static const std::vector< std::string > &getAvailableCodecs();

  private:
    bool sendFrame() const;
    static const AVCodec *getCodec(const std::string &codecName);

  private:
    int64_t m_frameCount = 0;

    priv::AVScoped< AVFormatContext > m_formatCtx;
    priv::AVScoped< AVCodecContext > m_codecCtx;
    priv::AVScoped< AVFrame > m_frame;
    priv::AVScoped< AVPacket > m_pkt;
    priv::AVScoped< SwsContext > m_swsCtx;
    priv::AVScoped< AVIOContext > m_avioCtx;

    // AVFormatContext* m_formatCtx = nullptr;
    // AVCodecContext* m_codecCtx = nullptr;
    AVStream* m_stream = nullptr;
    // AVFrame* m_frame = nullptr;
    // AVPacket* m_pkt = nullptr;
    // SwsContext* m_swsCtx = nullptr;

    sf::Texture m_texture;

    std::string m_metadataFilename;

    bool m_isRecording = false;

    sf::Clock m_clock;
    EventRecorder m_recorder;

    inline static std::vector< std::string > m_availableCodecs;
  };
} // namespace nx
