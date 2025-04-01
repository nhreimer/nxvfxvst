#pragma once

namespace nx
{
  struct ParticleLayoutData_t
  {
    sf::Color startColor { 255, 255, 255 };
    sf::Color endColor { 0, 0, 0 };

    sf::Color outlineColor { 255, 255, 255 };
    float outlineThickness { 0.f };

    float radius { 30.f };
    uint8_t shapeSides { 30 };      // the number of sides, e.g., 3 = triangle
    int32_t timeoutInMS { 1500 };

    float spreadMultiplier { 1.f };
    float jitterMultiplier { 0.f }; // 0 = no jitter
    sf::Vector2f positionOffset { 0.f, 0.f };

    float boostVelocity { 0.f };

    float velocitySizeMultiplier { 0.f }; // 0 = don't increase size based on velocity

    sf::BlendMode blendMode { sf::BlendNone };
  };
}