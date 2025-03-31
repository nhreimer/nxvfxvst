#pragma once

namespace nx
{

  struct TimedParticle
  {
    sf::CircleShape shape;

    int32_t timeLeft { 0 };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialColor { sf::Color::White };
  };

}