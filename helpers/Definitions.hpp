#pragma once

namespace nx
{
  // the number of channel pipelines to set up
  // the large the number, the more resources utilized
  constexpr int32_t MAX_MIDI_CHANNELS = 4;

  // the number of audio channels, usually one because stereo gets combined
  // but there might be additional virtual audio channels in the future
  constexpr int32_t MAX_AUDIO_CHANNELS = 1;

  constexpr int32_t MAX_CHANNELS = MAX_MIDI_CHANNELS + MAX_AUDIO_CHANNELS;

  // the refresh rate for getting the playhead
  constexpr float PLAYHEAD_INTERVAL_IN_SECS = 0.25f; // 0.25f = 250ms

  // the number of VST3 parameters to pre-allocate
  constexpr int32_t PARAMETERS_ENABLED = 256;

  // the average window size used for collecting rendering stats
  constexpr int32_t RENDER_AVERAGE_SECONDS = 3;

  // the max samples allowed for an average
  constexpr int32_t RENDER_SAMPLES_COUNT = 64;

  // audio window sizes
  constexpr size_t FFT_SIZE = 128;
  constexpr size_t FFT_BINS = FFT_SIZE / 2;

  // using AudioProcessorBuffer = std::array< float, FFT_SIZE >;
  using AudioDataBuffer = std::array< float, FFT_BINS >;
}