/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <nlohmann/json.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Glsl.hpp>

#include "models/easings/EasingsBase.hpp"
#include "models/InterfaceTypes.hpp"


// Customer JSON converters belong in this namespace
namespace nlohmann
{
  // sf::Glsl::Vec3 (alias for sf::Vector3<float>)
  inline void to_json(nlohmann::json& j, const sf::Glsl::Vec3& v)
  {
    j = nlohmann::json::array({ v.x, v.y, v.z });
  }

  inline void from_json(const nlohmann::json& j, sf::Glsl::Vec3& v)
  {
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
    j.at(2).get_to(v.z);
  }

  // sf::Vector2f
  inline void to_json(nlohmann::json& j, const sf::Vector2f& v)
  {
    j = nlohmann::json::array({ v.x, v.y });
  }

  inline void from_json(const nlohmann::json& j, sf::Vector2f& v)
  {
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
  }

  // sf::Color
  inline void to_json(nlohmann::json& j, const sf::Color& c)
  {
    j = nlohmann::json::array({ c.r, c.g, c.b, c.a });
  }

  inline void from_json(const nlohmann::json& j, sf::Color& c)
  {
    c.r = j.at(0).get<uint8_t>();
    c.g = j.at(1).get<uint8_t>();
    c.b = j.at(2).get<uint8_t>();
    c.a = j.at(3).get<uint8_t>();
  }

  inline void to_json(json& j, const sf::BlendMode& mode)
  {
    j = json{
              {"colorSrcFactor", mode.colorSrcFactor},
              {"colorDstFactor", mode.colorDstFactor},
              {"colorEquation",  mode.colorEquation},
              {"alphaSrcFactor", mode.alphaSrcFactor},
              {"alphaDstFactor", mode.alphaDstFactor},
              {"alphaEquation",  mode.alphaEquation}
    };
  }

  inline void from_json(const json& j, sf::BlendMode& mode)
  {
    j.at("colorSrcFactor").get_to(mode.colorSrcFactor);
    j.at("colorDstFactor").get_to(mode.colorDstFactor);
    j.at("colorEquation").get_to(mode.colorEquation);
    j.at("alphaSrcFactor").get_to(mode.alphaSrcFactor);
    j.at("alphaDstFactor").get_to(mode.alphaDstFactor);
    j.at("alphaEquation").get_to(mode.alphaEquation);
  }

}

namespace nx
{

  class SerialHelper
  {
  public:

    template < typename TEnum >
    static bool isTypeGood( const nlohmann::json& j, const TEnum pipelineType )
    {
      return j.contains( "type" ) &&
             pipelineType == deserializeEnum< TEnum >( j.at( "type" ) );
    }

    template <typename Enum>
    static constexpr auto serializeEnum( Enum e )
    {
      if constexpr ( std::is_same_v< Enum, E_LayoutType > )
      {
        if ( e == E_LayoutType::E_EmptyLayout ) return "EmptyLayout";
        if ( e == E_LayoutType::E_RandomLayout ) return "RandomLayout";
        if ( e == E_LayoutType::E_SpiralLayout ) return "SpiralLayout";
        if ( e == E_LayoutType::E_LissajousCurveLayout ) return "LissajousCurveLayout";
        if ( e == E_LayoutType::E_FractalRingLayout ) return "FractalRingLayout";
        if ( e == E_LayoutType::E_LSystemCurveLayout ) return "LSystemCurveLayout";
        if ( e == E_LayoutType::E_GoldenSpiralLayout ) return "GoldenSpiralLayout";
        if ( e == E_LayoutType::E_EllipticalLayout ) return "EllipticalLayout";
      }

      else if constexpr ( std::is_same_v< Enum, E_ModifierType > )
      {
        if ( e == E_ModifierType::E_SequentialModifier ) return "SequentialModifier";
        if ( e == E_ModifierType::E_FullMeshModifier ) return "FullMeshModifier";
        if ( e == E_ModifierType::E_RingZoneMeshModifier ) return "RingZoneMeshModifier";
        if ( e == E_ModifierType::E_PerlinDeformerModifier ) return "PerlinDeformerModifier";
        if ( e == E_ModifierType::E_MirrorModifier ) return "MirrorModifier";
      }

      else if constexpr ( std::is_same_v< Enum, E_ShaderType > )
      {
        switch ( e )
        {
          case E_ShaderType::E_GlitchShader: return "GlitchShader";
          case E_ShaderType::E_BlurShader: return "BlurShader";
          case E_ShaderType::E_RippleShader: return "RippleShader";
          case E_ShaderType::E_StrobeShader: return "StrobeShader";
          case E_ShaderType::E_RumbleShader: return "RumbleShader";
          case E_ShaderType::E_KaleidoscopeShader: return "KaleidoscopeShader";
          case E_ShaderType::E_SmearShader: return "SmearShader";
          case E_ShaderType::E_DensityHeatMapShader: return "DensityHeatMapShader";
          case E_ShaderType::E_FeedbackShader: return "FeedbackShader";
          case E_ShaderType::E_DualKawaseBlurShader: return "DualKawaseBlurShader";
          case E_ShaderType::E_TransformShader: return "TransformShader";
          case E_ShaderType::E_ColorShader: return "ColorShader";
          default: break;
        }
      }

      else if constexpr ( std::is_same_v< Enum, E_BehaviorType > )
      {
        switch ( e )
        {
          case E_BehaviorType::E_JitterBehavior: return "JitterBehavior";
          case E_BehaviorType::E_FreeFallBehavior: return "FreeFallBehavior";
          case E_BehaviorType::E_MagneticBehavior: return "MagneticBehavior";
          default: break;
        }
      }

      return "UNKNOWN";
    }

    ///////////////////////////////////////////////////////////////////////////
    template < typename TEnum >
    static constexpr auto deserializeEnum( const std::string& typeName )
    {
      if constexpr ( std::is_same_v< TEnum, E_LayoutType > )
      {
        if ( typeName == "EmptyLayout" ) return E_LayoutType::E_EmptyLayout;
        if ( typeName == "RandomLayout" ) return E_LayoutType::E_RandomLayout;
        if ( typeName == "SpiralLayout" ) return E_LayoutType::E_SpiralLayout;
        if ( typeName == "FractalRingLayout" ) return E_LayoutType::E_FractalRingLayout;
        if ( typeName == "LissajousCurveLayout" ) return E_LayoutType::E_LissajousCurveLayout;
        if ( typeName == "LSystemCurveLayout" ) return E_LayoutType::E_LSystemCurveLayout;
        if ( typeName == "GoldenSpiralLayout" ) return E_LayoutType::E_GoldenSpiralLayout;
        if ( typeName == "EllipticalLayout" ) return E_LayoutType::E_EllipticalLayout;
        return E_LayoutType::E_EmptyLayout;
      }
      else if constexpr ( std::is_same_v< TEnum, E_ShaderType > )
      {
        if ( typeName == "GlitchShader" ) return E_ShaderType::E_GlitchShader;
        if ( typeName == "BlurShader" ) return E_ShaderType::E_BlurShader;
        if ( typeName == "StrobeShader" ) return E_ShaderType::E_StrobeShader;
        if ( typeName == "RippleShader" ) return E_ShaderType::E_RippleShader;
        if ( typeName == "KaleidoscopeShader" ) return E_ShaderType::E_KaleidoscopeShader;
        if ( typeName == "RumbleShader" ) return E_ShaderType::E_RumbleShader;
        if ( typeName == "SmearShader" ) return E_ShaderType::E_SmearShader;
        if ( typeName == "DensityHeatMapShader" ) return E_ShaderType::E_DensityHeatMapShader;
        if ( typeName == "FeedbackShader" ) return E_ShaderType::E_FeedbackShader;
        if ( typeName == "DualKawaseBlurShader" ) return E_ShaderType::E_DualKawaseBlurShader;
        if ( typeName == "TransformShader" ) return E_ShaderType::E_TransformShader;
        if ( typeName == "ColorShader" ) return E_ShaderType::E_ColorShader;
        return E_ShaderType::E_InvalidShader;
      }
      else if constexpr ( std::is_same_v< TEnum, E_ModifierType > )
      {
        if ( typeName == "SequentialModifier" ) return E_ModifierType::E_SequentialModifier;
        if ( typeName == "FullMeshModifier" ) return E_ModifierType::E_FullMeshModifier;
        if ( typeName == "RingZoneMeshModifier" ) return E_ModifierType::E_RingZoneMeshModifier;
        if ( typeName == "PerlinDeformerModifier" ) return E_ModifierType::E_PerlinDeformerModifier;
        if ( typeName == "MirrorModifier" ) return E_ModifierType::E_MirrorModifier;
        return E_ModifierType::E_InvalidModifier;
      }
      else if constexpr ( std::is_same_v< TEnum, E_BehaviorType > )
      {
        if ( typeName == "JitterBehavior" ) return E_BehaviorType::E_JitterBehavior;
        if ( typeName == "FreeFallBehavior" ) return E_BehaviorType::E_FreeFallBehavior;
        if ( typeName == "MagneticBehavior" ) return E_BehaviorType::E_MagneticBehavior;
        return E_BehaviorType::E_InvalidBehavior;
      }
    }

    static std::string convertBlendModeToString( const sf::BlendMode& mode )
    {
      if ( mode == sf::BlendAdd ) return "BlendAdd";
      if ( mode == sf::BlendAlpha ) return "BlendAlpha";
      if ( mode == sf::BlendMax ) return "BlendMax";
      if ( mode == sf::BlendMin ) return "BlendMin";
      if ( mode == sf::BlendMax ) return "BlendMax";
      if ( mode == sf::BlendMultiply ) return "BlendMultiply";
      if ( mode == sf::BlendNone ) return "BlendNone";
      return "BlendAdd";
    }

    static sf::BlendMode convertBlendModeFromString( const std::string& s )
    {
      if (s == "BlendAdd") return sf::BlendAdd;
      if (s == "BlendAlpha") return sf::BlendAlpha;
      if (s == "BlendMax") return sf::BlendMax;
      if (s == "BlendMin") return sf::BlendMin;
      if (s == "BlendMultiply") return sf::BlendMultiply;
      if (s == "BlendNone") return sf::BlendNone;

      return sf::BlendAdd;
    }

    template < typename T >
    static nlohmann::json convertVectorToJson( const sf::Vector2< T >& vec )
    {
      return { vec.x, vec.y };
    }

    template < typename T >
    static sf::Vector2< T > convertVectorFromJson(const nlohmann::json& j, sf::Vector2< T > fallback = { 0, 0 } )
    {
      if (!j.is_array() || j.size() != 2) return fallback;

      return sf::Vector2< T >( j[0].get< T >(), j[1].get< T >() );
    }

    static nlohmann::json convertColorToJson( const sf::Color& color )
    {
      return
      {
        { "r", color.r },
        { "g", color.g },
        { "b", color.b },
        { "a", color.a }
      };
    }

    static sf::Color convertColorFromJson( const nlohmann::json& j, sf::Color defaultColor = sf::Color::White )
    {
      return sf::Color(
          j.value("r", defaultColor.r),
          j.value("g", defaultColor.g),
          j.value("b", defaultColor.b),
          j.value("a", defaultColor.a)
      );
    }

    template < typename TValue >
    static std::string convertVec2ToJSON( const sf::Vector2< TValue >& vec )
    {
      return
      {
        { "x", vec.x },
        { "y", vec.y }
      };
    }

    template < typename TValue >
    static sf::Vector2< TValue > convertVec2FromJSON( const nlohmann::json& j )
    {
      return sf::Vector2< TValue > { j.value( "x", 0 ), j.value( "y", 0 ) };
    }
  };

}