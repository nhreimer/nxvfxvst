#pragma once

#include "models/encoder/RawRGBAEncoder.hpp"
#include "models/encoder/FFMpegEncoder.hpp"

namespace nx
{
  enum class E_Encoder : int8_t
  {
    E_RawRGBA,
    E_MP4
  };

  struct EncoderFactory
  {
    static std::unique_ptr< IEncoder > create( const E_Encoder encoderType, const EncoderData_t& data )
    {
      switch ( encoderType )
      {
        case E_Encoder::E_RawRGBA:
          return std::make_unique< RawRGBAEncoder >( data );

        case E_Encoder::E_MP4:
          return std::make_unique< FFMpegEncoder >( data );

        default:
          return nullptr;
      }
    }
  };
}