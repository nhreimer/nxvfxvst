#pragma once

namespace nx
{
  struct ParticleLayoutData_t
  {
    // // the original position assigned by the layout engine
    // sf::Vector2f layoutOriginalPosition;

    // the type of blend mode for the particles
    sf::BlendMode blendMode { sf::BlendNone };
  };
}