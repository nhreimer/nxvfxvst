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
  /// Holds the following info:
  /// * Scene-level read-only state
  /// * Rhythmic/time context (e.g. BPM, GlobalTime)
  /// * View, UI toggles, etc.
  struct GlobalInfo_t
  {
    // this size of the child window inside the parent in plugin mode
    // otherwise the main window size in standalone mode
    sf::Vector2u windowSize;

    // this is required for so many things that it
    // makes sense to hold here rather than calculate it
    // in numerous components per frame
    sf::Vector2f windowHalfSize;

    // the global view to be used when SFML displays at the end of
    // the rendering chain
    sf::View windowView;

    // hides all the menus
    bool hideMenu { false };

    // gets written by the processor
    // gets read by the controller
    double bpm { 0.f };

    // get written by the processor
    // gets read by the controller
    double playhead { 0.f };

    // All pipelines and shaders have access to:
    // * Consistent global timebase (for rhythms, perlin offsets, etc.)
    // * Frame-specific info (e.g., strobe every N frames)

    // total runtime
    float elapsedTimeSeconds { 0.f };

    // total frames processed
    std::uint64_t frameCount { 0 };

    // this defaults to 48kHz, but it comes from the processor on plugin startup
    float sampleRate { 48000.f };
  };
}