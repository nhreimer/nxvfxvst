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
    std::string mp4PresetOption { "p4" };

    // Tunes:
    //    hq = high quality
    //    ull = ultra-low latency (great for streaming)
    //    lossless = no compression
    std::string mp4TuningOption { "hq" };
  };

  struct IEncoder
  {
    virtual ~IEncoder() = default;
    virtual void writeFrame( const sf::RenderWindow& texture ) = 0;
    virtual bool isRecording() const = 0;
  };

}