#pragma once

namespace nx
{
  struct ColorHelper
  {

    static sf::Color lerpColor(const sf::Color &a, const sf::Color &b, float t)
    {
      t = std::clamp(t, 0.f, 1.f);
      return sf::Color(static_cast< uint8_t >(a.r + (b.r - a.r) * t), static_cast< uint8_t >(a.g + (b.g - a.g) * t),
                       static_cast< uint8_t >(a.b + (b.b - a.b) * t), static_cast< uint8_t >(a.a + (b.a - a.a) * t));
    }

    ///
    /// @param colorDestination
    /// @param colorCurrent
    /// @param timeStartInSeconds
    /// @param timeEndInSeconds
    /// @param timeCurrentInSeconds
    /// @return the current color
    static sf::Color reverseLerpColor(
      const sf::Color& colorDestination,
      const sf::Color& colorCurrent,
      const float timeStartInSeconds,
      const float timeEndInSeconds,
      const float timeCurrentInSeconds)
    {
      const float progress = std::clamp(
        (timeCurrentInSeconds - timeStartInSeconds) /
            (timeEndInSeconds - timeStartInSeconds),
            0.f,
            1.f);

      if (progress >= 1.f) return colorDestination;

      auto solve = [&](const uint8_t c3_chan, const uint8_t c2_chan) -> uint8_t
      {
        const float num = static_cast<float>(c3_chan) - progress * static_cast<float>(c2_chan);
        const float denom = 1.f - progress;
        return static_cast<uint8_t>(std::clamp(num / denom, 0.f, 255.f));
      };

      return sf::Color(
        solve(colorCurrent.r, colorDestination.r),
        solve(colorCurrent.g, colorDestination.g),
        solve(colorCurrent.b, colorDestination.b),
        solve(colorCurrent.a, colorDestination.a)
      );
    }


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
                                   const double percentage )
    {
      return sf::Color
      {
        getColorDelta( fromColor.r, toColor.r, percentage ),
        getColorDelta( fromColor.g, toColor.g, percentage ),
        getColorDelta( fromColor.b, toColor.b, percentage ),
        getColorDelta( fromColor.a, toColor.a, percentage )
        };
    }

    static uint8_t getColorDelta( uint8_t fromColorBand, uint8_t toColorBand, const double percentage )
    {
      return static_cast< uint8_t >(static_cast< double >(toColorBand - fromColorBand) * percentage) + fromColorBand;
    }

    static sf::Color getColorPercentage( const sf::Color & maxColor, const float percentage )
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