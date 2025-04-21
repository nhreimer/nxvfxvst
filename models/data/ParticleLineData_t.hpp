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

    bool useParticleColors { false };
    sf::Color lineColor = sf::Color(255, 255, 255, 100);
    sf::Color otherLineColor = sf::Color(255, 255, 255, 100);

    // TODO: this is probably not in use anymore.
    // TODO: ModifierPipeline has a global modifier blend option that seems
    // TODO: to be used
    sf::BlendMode blendMode { sf::BlendNone };
  };

}