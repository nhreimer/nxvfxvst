#pragma once

namespace nx
{
  struct Midi_t
  {
    int16_t channel { 0 };
    int16_t pitch { 0 };
    float velocity { 0.f };
    sf::BlendMode blendMode { sf::BlendAdd };
  };
}