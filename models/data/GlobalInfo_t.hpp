#pragma once

namespace nx
{
  struct GlobalInfo_t
  {
    sf::Vector2u windowSize;
    sf::View windowView;
    bool hideMenu { false };

    // gets written by the processor
    // gets read by the controller
    double bpm { 0.f };
  };
}