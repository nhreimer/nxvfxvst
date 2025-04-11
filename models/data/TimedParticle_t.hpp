#pragma once

#include <SFML/Graphics/CircleShape.hpp>

namespace nx
{

  struct TimedParticle_t
  {
    sf::CircleShape shape;

    int32_t timeLeft { 0 };

    // time in seconds when particle was created
    float spawnTime { 0.f };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialColor { sf::Color::White };
  };

}