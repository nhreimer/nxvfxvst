#pragma once

namespace nx
{
  struct GlobalInfo_t
  {
    sf::Vector2u windowSize;
    sf::View windowView;
    bool hideMenu { false };
  };
}