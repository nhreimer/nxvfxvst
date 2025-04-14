#pragma once

namespace nx
{
  struct ColorHelper
  {

    static sf::Color getNextColor( const sf::Color& fromColor,
                                          const sf::Color& toColor,
                                          double percentage )
    {
      return sf::Color
      {
        getColorDelta( fromColor.r, toColor.r, percentage ),
        getColorDelta( fromColor.g, toColor.g, percentage ),
        getColorDelta( fromColor.b, toColor.b, percentage ),
        getColorDelta( fromColor.a, toColor.a, percentage )
        };
    }

    static uint8_t getColorDelta( uint8_t fromColorBand, uint8_t toColorBand, double percentage )
    {
      return ( uint8_t )( ( double )( toColorBand - fromColorBand ) * percentage ) + fromColorBand;
    }

    static sf::Color getColorPercentage( const sf::Color & maxColor, float percentage )
    {
      return sf::Color(
      ( int8_t )( ( float )maxColor.r * percentage ),
      ( int8_t )( ( float )maxColor.g * percentage ),
      ( int8_t )( ( float )maxColor.b * percentage ),
      ( int8_t )( ( float )maxColor.a * percentage )
      );
    }

    static sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t)
    {
      t = std::clamp(t, 0.f, 1.f);
      return sf::Color(
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
      );
    }


  };
}