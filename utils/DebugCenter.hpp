#pragma once

namespace nx
{
  struct DebugCenter
  {
    DebugCenter()
    {
      debugCenter.setFillColor( sf::Color::Red );
      const auto& bounds = debugCenter.getGlobalBounds();
      debugCenter.setOrigin( { bounds.size.x / 2, bounds.size.y / 2 } );
    }

    sf::CircleShape debugCenter { 2.f };
  };
}