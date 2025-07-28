/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include "models/encoder/RawRGBAEncoder.hpp"
// #include "models/encoder/FFMpegEncoder.hpp"

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

        // case E_Encoder::E_MP4:
        //   return std::make_unique< FFMpegEncoder >( data );

        default:
          return nullptr;
      }
    }
  };
}