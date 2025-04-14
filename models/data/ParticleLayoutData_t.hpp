#pragma once

#include "helpers/SerialHelper.hpp"

#include <helpers/MenuHelper.hpp>

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

    //float spreadMultiplier { 1.f };
    //float jitterMultiplier { 0.f }; // 0 = no jitter
    //sf::Vector2f positionOffset { 0.f, 0.f };

    float boostVelocity { 0.f };

    float velocitySizeMultiplier { 0.f }; // 0 = don't increase size based on velocity

    sf::BlendMode blendMode { sf::BlendNone };

  };

  class ParticleHelper
  {
  public:

    static nlohmann::json serialize( const ParticleLayoutData_t& data, std::string_view typeName )
    {
      return
   {
      { "type", typeName },
      { "startColor", SerialHelper::convertColorToJson( data.startColor ) },
      { "endColor", SerialHelper::convertColorToJson( data.endColor ) },
      { "outlineColor", SerialHelper::convertColorToJson( data.outlineColor ) },
      { "outlineThickness", data.outlineThickness },
      { "radius", data.radius },
      { "shapeSides", data.shapeSides },
      { "timeoutInMS", data.timeoutInMS },
      { "boostVelocity", data.boostVelocity },
      { "velocitySizeMultiplier", data.velocitySizeMultiplier },
      { "blendMode", SerialHelper::convertBlendModeToString( data.blendMode ) }
      };
    }

    static void deserialize( ParticleLayoutData_t& data, const nlohmann::json & j )
    {
      data.startColor = SerialHelper::convertColorFromJson(j.at("startColor"), sf::Color::White);
      data.endColor = SerialHelper::convertColorFromJson(j.at("endColor"), sf::Color::Black);
      data.outlineColor = SerialHelper::convertColorFromJson(j.at("outlineColor"), sf::Color::White);
      data.outlineThickness = j.value("outlineThickness", 0.f);
      data.radius = j.value("radius", 30.f);
      data.shapeSides = j.value("shapeSides", 30);
      data.timeoutInMS = j.value("timeoutInMS", 1500);
      data.boostVelocity = j.value("boostVelocity", 0.f);
      data.velocitySizeMultiplier = j.value("velocitySizeMultiplier", 0.f);
      data.blendMode = SerialHelper::convertBlendModeFromString(j.value("blendMode", "None"));
    }

    static void drawMenu( ParticleLayoutData_t & data )
    {
      drawAppearanceMenu( data );
      drawAdjustmentMenu( data );
    }

  private:

    static void drawAppearanceMenu( ParticleLayoutData_t & data )
    {
      if ( ImGui::TreeNode( "Particle Appearance" ) )
      {
        ImVec4 color = data.startColor;

        if ( ImGui::ColorPicker4( "Particle Fill##1",
                                  reinterpret_cast< float * >( &color ),
                                  ImGuiColorEditFlags_AlphaBar,
                                  nullptr ) )
        {
          data.startColor = color;
        }

        ImGui::Separator();
        ImGui::SliderFloat( "Thickness##2", &data.outlineThickness, 0.f, 25.f );

        ImVec4 outlineColor = data.outlineColor;

        if ( ImGui::ColorPicker4( "Particle Outline##1",
                                  reinterpret_cast< float * >( &outlineColor ),
                                  ImGuiColorEditFlags_AlphaBar,
                                  nullptr ) )
        {
          data.outlineColor = outlineColor;
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    static void drawAdjustmentMenu( ParticleLayoutData_t & data )
    {
      if ( ImGui::TreeNode( "Particle Adjust" ) )
      {
        int32_t sides = data.shapeSides;
        if ( ImGui::SliderInt( "Sides##1", &sides, 3, 30 ) ) data.shapeSides = sides;
        ImGui::SliderFloat( "Radius##1", &data.radius, 1.0f, 100.0f );
        ImGui::SliderInt( "Timeout##1", &data.timeoutInMS, 15, 10000 );
        ImGui::SliderFloat( "Boost##1", &data.boostVelocity, 0.f, 1.f );
        ImGui::SliderFloat( "Velocity Size Mult##1", &data.velocitySizeMultiplier, 0.f, 50.f );

        MenuHelper::drawBlendOptions( data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }
  };
}