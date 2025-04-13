#pragma once

namespace nx
{
  struct ParticleLineData_t
  {
    // sf::Color startColor { 255, 255, 255 };
    // sf::Color endColor { 0, 0, 0 };
    bool isActive{ true };
    float lineThickness{ 2.f };
    float swellFactor { 1.5f  };
    float easeDownInSeconds { 1.f };
    // add is the default given the default shader and texture usage
    sf::BlendMode blendMode { sf::BlendNone };
  };

}