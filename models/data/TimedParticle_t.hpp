#pragma once

#include <SFML/Graphics/CircleShape.hpp>

namespace nx
{

  struct TimedParticle_t
  {
    sf::CircleShape shape;

    int32_t timeLeft { 0 };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialColor { sf::Color::White };
  };

}