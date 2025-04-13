#pragma once

#include <SFML/Graphics/CircleShape.hpp>

namespace nx
{

  struct TimedParticle_t
  {
    TimedParticle_t() = default;

    TimedParticle_t( const TimedParticle_t& other )
      : shape( other.shape ),
        timeLeft( other.timeLeft ),
        spawnTime( other.spawnTime ), // must keep the time the same or uncoordinated events occur
        initialColor( other.initialColor )
    {}

    TimedParticle_t& operator=( const TimedParticle_t& other )
    {
      shape = other.shape;
      timeLeft = other.timeLeft;
      spawnTime = other.spawnTime;
      initialColor = other.initialColor;
      return *this;
    }

    sf::CircleShape shape;

    int32_t timeLeft { 0 };

    // time in seconds when particle was created
    float spawnTime { 0.f };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialColor { sf::Color::White };
  };

}