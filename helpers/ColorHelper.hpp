#pragma once

namespace nx
{
  struct ColorHelper
  {

    static constexpr auto toVec4(const sf::Color& c)
    {
      return sf::Glsl::Vec4(
          c.r / 255.f,
          c.g / 255.f,
          c.b / 255.f,
          c.a / 255.f
      );
    }

    static sf::Glsl::Vec3 convertFromVec4(const sf::Color& c)
    {
      return sf::Glsl::Vec3( c.r / 255.f, c.g / 255.f, c.b / 255.f );
    }

    static void drawImGuiColorEdit4( const std::string& label, sf::Color& color )
    {
      ImVec4 imColor = color;
      if ( ImGui::ColorEdit4( label.c_str(), reinterpret_cast< float * >( &imColor ) ) )
        color = imColor;
    }

    static void drawImGuiColorEdit3( const std::string& label, sf::Color& color )
    {
      ImVec4 imColor = color;
      if ( ImGui::ColorEdit3( label.c_str(), reinterpret_cast< float * >( &imColor ) ) )
      {
        color = imColor;
        color.a = 255.f;
      }
    }

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