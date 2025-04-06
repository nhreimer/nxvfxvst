#pragma once

namespace nx
{
  struct GlobalInfo_t
  {
    sf::Vector2u windowSize;
    sf::View windowView;
    bool hideMenu { false };

    // used for showing positions on the screen
    int16_t cursorDisappearsAfterSeconds { 5 };
    ImColor cursorColor { 127, 127, 127, 255 };
  };
}