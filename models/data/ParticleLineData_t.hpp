#pragma once

namespace nx
{
  struct ParticleLineData_t
  {
    // sf::Color startColor { 255, 255, 255 };
    // sf::Color endColor { 0, 0, 0 };
    bool useConnectors{ true };
    float lineThickness{ 2.f };
    // add is the default given the default shader and texture usage
    sf::BlendMode blendMode { sf::BlendNone };
  };

}