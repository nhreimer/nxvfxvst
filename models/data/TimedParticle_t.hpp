#pragma once

#include "shapes/CircleParticle.hpp"

namespace nx
{

  struct TimedParticle_t
  {
    TimedParticle_t() = default;

    TimedParticle_t( const TimedParticle_t& other )
      : shape( other.shape ),
        timeLeft( other.timeLeft ),
        spawnTime( other.spawnTime ), // must keep the time the same or uncoordinated events occur
        initialStartColor( other.initialStartColor ),
        initialEndColor( other.initialEndColor ),
        originalPosition( other.originalPosition )
    {}

    TimedParticle_t& operator=( const TimedParticle_t& other )
    {
      shape = other.shape;
      timeLeft = other.timeLeft;
      spawnTime = other.spawnTime;
      initialStartColor = other.initialStartColor;
      initialEndColor = other.initialEndColor;
      originalPosition = other.originalPosition;
      return *this;
    }

    //Particle shape;
    IParticle * shape;

    int32_t timeLeft { 0 };

    // time in seconds when particle was created
    float spawnTime { 0.f };

    // this is the initial color. it shouldn't change once set.
    sf::Color initialStartColor { sf::Color::White };
    sf::Color initialEndColor { sf::Color::Black };

    // provided by layout
    sf::Vector2f originalPosition;
  };

}