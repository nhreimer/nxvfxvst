#pragma once

#include "../easings/TimeEasing.hpp"

namespace nx
{
  // holds info ONLY related to the particle
  struct ParticleData_t
  {
    // and you can apply easings to it
    E_EasingType colorEasing { E_EasingType::E_Linear };

    float outlineThickness { 0.f };

    float radius { 30.f };
    uint8_t pointCount { 30 };

    float velocitySizeMultiplier { 1.f };
    float boostVelocity { 0.f };

    float timeoutInSeconds { 0.75f };

    sf::Color fillStartColor { sf::Color::White };
    sf::Color fillEndColor { sf::Color::White };

    sf::Color outlineStartColor { sf::Color::White };
    sf::Color outlineEndColor { sf::Color::White };
  };

}