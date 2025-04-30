#pragma once

namespace nx
{

  struct EncoderData_t
  {
    // screen size
    sf::Vector2u size;
    int32_t fps { 60 };
    std::array< char, 256 > outputFilename;

    // Presets:
    //    p1 = fastest, lowest quality
    //    p7 = slowest, best quality (still blazing fast on your GPUs)
    std::string mp4PresetOption { "p7" }; // p4 is the "normal"

    // Tunes:
    //    hq = high quality
    //    ull = ultra-low latency (great for streaming)
    //    lossless = no compression
    std::string mp4TuningOption { "hq" };
  };

  struct IEncoder
  {
    virtual ~IEncoder() = default;

    /// Called every frame
    /// @param texture the texture to write
    virtual void writeFrame( const sf::RenderWindow& texture ) = 0;

    [[nodiscard]]
    virtual bool isRecording() const = 0;

    // this is used for synchronizing the video and midi events
    // it'll create a map and then dump the map out for you along with other metadata
    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;
  };

}