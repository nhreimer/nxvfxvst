#pragma once

#include "helpers/SerialHelper.hpp"

namespace nx
{
  struct ParticleLayoutData_t
  {
    sf::Color startColor { 255, 255, 255 };
    sf::Color endColor { 0, 0, 0 };

    sf::Color outlineColor { 255, 255, 255 };
    float outlineThickness { 0.f };

    float radius { 30.f };
    uint8_t shapeSides { 30 };      // the number of sides, e.g., 3 = triangle
    int32_t timeoutInMS { 1500 };

    float spreadMultiplier { 1.f };
    float jitterMultiplier { 0.f }; // 0 = no jitter
    sf::Vector2f positionOffset { 0.f, 0.f };

    float boostVelocity { 0.f };

    float velocitySizeMultiplier { 0.f }; // 0 = don't increase size based on velocity

    sf::BlendMode blendMode { sf::BlendNone };

    nlohmann::json serialize( std::string_view typeName ) const
    {
      return
      {
          { "type", typeName },
          { "startColor", SerialHelper::convertColorToJson( startColor ) },
          { "endColor", SerialHelper::convertColorToJson( endColor ) },
          { "outlineColor", SerialHelper::convertColorToJson( outlineColor ) },
          { "outlineThickness", outlineThickness },
          { "radius", radius },
          { "shapeSides", shapeSides },
          { "timeoutInMS", timeoutInMS },
          { "spreadMultiplier", spreadMultiplier },
          { "jitterMultiplier", jitterMultiplier },
          { "positionOffset", SerialHelper::convertVectorToJson( positionOffset ) },
          { "boostVelocity", boostVelocity },
          { "velocitySizeMultiplier", velocitySizeMultiplier },
          { "blendMode", SerialHelper::convertBlendModeToString( blendMode ) }
      };
    }

    void deserialize( const nlohmann::json & j )
    {
      startColor = SerialHelper::convertColorFromJson(j.at("startColor"), sf::Color::White);
      endColor = SerialHelper::convertColorFromJson(j.at("endColor"), sf::Color::Black);
      outlineColor = SerialHelper::convertColorFromJson(j.at("outlineColor"), sf::Color::White);
      outlineThickness = j.value("outlineThickness", 0.f);
      radius = j.value("radius", 30.f);
      shapeSides = j.value("shapeSides", 30);
      timeoutInMS = j.value("timeoutInMS", 1500);
      spreadMultiplier = j.value("spreadMultiplier", 1.f);
      jitterMultiplier = j.value("jitterMultiplier", 0.f);
      positionOffset = SerialHelper::convertVectorFromJson< float >(j.at("positionOffset"));
      boostVelocity = j.value("boostVelocity", 0.f);
      velocitySizeMultiplier = j.value("velocitySizeMultiplier", 0.f);
      blendMode = SerialHelper::convertBlendModeFromString(j.value("blendMode", "None"));
    }
  };
}