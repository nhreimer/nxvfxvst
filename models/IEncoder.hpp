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

namespace nx
{

  struct EncoderData_t
  {
    // screen size
    sf::Vector2u size;
    int32_t fps { 60 };
    std::array< char, 256 > outputFilename;

    // the playhead hits this time then it should start
    float startAtInSeconds { 0.f };

    //std::string codecName;

    // Presets:
    //    p1 = fastest, lowest quality
    //    p7 = slowest, best quality (still blazing fast on your GPUs)
    //std::string mp4PresetOption { "p7" }; // p4 is the "normal"

    // Tunes:
    //    hq = high quality
    //    ull = ultra-low latency (great for streaming)
    //    lossless = no compression
    //std::string mp4TuningOption { "hq" };
  };

  struct IEncoder
  {
    virtual ~IEncoder() = default;

    /// Called every frame
    /// @param texture the texture to write
    virtual void writeFrame( const double playhead, const sf::RenderWindow& texture ) = 0;

    [[nodiscard]]
    virtual bool isRecording() const = 0;

    // this is used for synchronizing the video and midi events
    // it'll create a map and then dump the map out for you along with other metadata
    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;

  };

}