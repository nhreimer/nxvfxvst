#pragma once

namespace nx
{

  struct Midi_t
  {
    int16_t channel { 0 };
    int16_t pitch { 0 };
    float velocity { 0.f };
  };

  struct MidiVisNode_t
  {
    sf::CircleShape shape;
    int32_t timeRemaining { 0 };
    sf::Color initialColor;
    uint32_t id { 0 };
  };

  struct GlobalData_t
  {
    sf::View view;
    uint8_t activeChannel { 0 };
    sf::Vector2u windowSize { 0, 0 };
    bool isMenuHidden { false };
    sf::Color mainBackgroundColor { sf::Color::Black };
  };

  struct ChannelOptionsData
  {
    bool isMuted { false };
  };

  struct GlobalChannelData
  {
    sf::BlendMode blendMode { sf::BlendAdd };
  };

  struct ChannelObjectData
  {
    sf::Color startColor { 255, 255, 255 };
    sf::Color endColor { 0, 0, 0 };

    sf::Color outlineColor { 255, 255, 255 };
    float outlineThickness { 0.f };

    float radius { 15.f };
    uint8_t shapeSides { 30 };      // the number of sides, e.g., 3 = triangle
    int32_t timeoutInMS { 1500 };

    float spreadMultiplier { 1.f };
    float jitterMultiplier { 0.f }; // 0 = no jitter
    sf::Vector2f positionOffset { 0.f, 0.f };

    float boostVelocity { 0.f };

    // sf::BlendMode blendMode { sf::BlendAdd };
  };

  struct ChannelBlurData
  {
    float blurHorizontal { 0.f };
    float blurVertical { 0.f };
  };

  struct ChannelLineData
  {
    // sf::Color startColor { 255, 255, 255 };
    // sf::Color endColor { 0, 0, 0 };
    bool useConnectors{ false };
    float lineThickness{ 2.f };
    bool drawBefore { false };
    // add is the default given the default shader and texture usage
    sf::BlendMode blendMode { sf::BlendMax };
  };



}