#pragma once

namespace nx
{
  struct ColorHelper
  {

    static inline sf::Color getNextColor( const sf::Color& fromColor,
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

    static inline uint8_t getColorDelta( uint8_t fromColorBand, uint8_t toColorBand, double percentage )
    {
      return ( uint8_t )( ( double )( toColorBand - fromColorBand ) * percentage ) + fromColorBand;
    }

    static inline sf::Color getColorPercentage( const sf::Color & maxColor, float percentage )
    {
      return sf::Color(
      ( int8_t )( ( float )maxColor.r * percentage ),
      ( int8_t )( ( float )maxColor.g * percentage ),
      ( int8_t )( ( float )maxColor.b * percentage ),
      ( int8_t )( ( float )maxColor.a * percentage )
      );
    }

  };
}