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

    bool useParticleColors { true };
    sf::Color lineColor = sf::Color(255, 255, 255, 255);
    sf::Color otherLineColor = sf::Color(255, 255, 255, 255);

    float curvature { 0.25f };
    int32_t lineSegments { 20 };
  };

}