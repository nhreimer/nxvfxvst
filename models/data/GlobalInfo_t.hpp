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