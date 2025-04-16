#pragma once

#include <nlohmann/json.hpp>

#include "models/InterfaceTypes.hpp"

namespace nx
{

  class SerialHelper
  {
  public:

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
      }

      else if constexpr ( std::is_same_v< Enum, E_ModifierType > )
      {
        if ( e == E_ModifierType::E_SequentialModifier ) return "SequentialModifier";
        if ( e == E_ModifierType::E_FullMeshModifier ) return "FullMeshModifier";
        if ( e == E_ModifierType::E_RingZoneMeshModifier ) return "RingZoneMeshModifier";
        if ( e == E_ModifierType::E_PerlinDeformerModifier ) return "PerlinDeformerModifier";
      }

      else if constexpr ( std::is_same_v< Enum, E_ShaderType > )
      {
        switch ( e )
        {
          case E_ShaderType::E_GlitchShader: return "GlitchShader";
          case E_ShaderType::E_BlurShader: return "BlurShader";
          case E_ShaderType::E_PulseShader: return "PulseShader";
          case E_ShaderType::E_RippleShader: return "RippleShader";
          case E_ShaderType::E_StrobeShader: return "StrobeShader";
          case E_ShaderType::E_RumbleShader: return "RumbleShader";
          case E_ShaderType::E_KaleidoscopeShader: return "KaleidoscopeShader";
          case E_ShaderType::E_SmearShader: return "SmearShader";
          default: break;
        }
      }

      else if constexpr ( std::is_same_v< Enum, E_BehaviorType > )
      {
        switch ( e )
        {
          case E_BehaviorType::E_JitterBehavior: return "JitterBehavior";
          case E_BehaviorType::E_FreeFallBehavior: return "FreeFallBehavior";
          case E_BehaviorType::E_RadialSpreaderBehavior: return "RadialSpreaderBehavior";
          case E_BehaviorType::E_ColorMorphBehavior: return "ColorMorphBehavior";
          case E_BehaviorType::E_MagneticBehavior: return "MagneticBehavior";
          default: break;
        }
      }

      return "UNKNOWN";
    }

    static E_LayoutType convertStringToLayoutType( const std::string& layoutType )
    {
      if ( layoutType == "EmptyLayout" ) return E_LayoutType::E_EmptyLayout;
      if ( layoutType == "RandomLayout" ) return E_LayoutType::E_RandomLayout;
      if ( layoutType == "SpiralLayout" ) return E_LayoutType::E_SpiralLayout;
      if ( layoutType == "FractalRingLayout" ) return E_LayoutType::E_FractalRingLayout;
      if ( layoutType == "LSystemCurveLayout" ) return E_LayoutType::E_LSystemCurveLayout;
      if ( layoutType == "GoldenSpiralLayout" ) return E_LayoutType::E_GoldenSpiralLayout;
      return E_LayoutType::E_EmptyLayout;
    }

    static E_ShaderType convertStringToShaderType( const std::string& shaderType )
    {
      if ( shaderType == "GlitchShader" ) return E_ShaderType::E_GlitchShader;
      if ( shaderType == "PulseShader" ) return E_ShaderType::E_PulseShader;
      if ( shaderType == "BlurShader" ) return E_ShaderType::E_BlurShader;
      if ( shaderType == "StrobeShader" ) return E_ShaderType::E_StrobeShader;
      if ( shaderType == "RippleShader" ) return E_ShaderType::E_RippleShader;
      if ( shaderType == "KaleidoscopeShader" ) return E_ShaderType::E_KaleidoscopeShader;
      if ( shaderType == "RumbleShader" ) return E_ShaderType::E_RumbleShader;
      if ( shaderType == "SmearShader" ) return E_ShaderType::E_SmearShader;
      return E_ShaderType::E_InvalidShader;
    }

    static E_ModifierType convertStringToModifierType( const std::string& modifierType )
    {
      if ( modifierType == "SequentialModifier" ) return E_ModifierType::E_SequentialModifier;
      if ( modifierType == "FullMeshModifier" ) return E_ModifierType::E_FullMeshModifier;
      if ( modifierType == "RingZoneMeshModifier" ) return E_ModifierType::E_RingZoneMeshModifier;
      if ( modifierType == "PerlinDeformerModifier" ) return E_ModifierType::E_PerlinDeformerModifier;
      return E_ModifierType::E_InvalidModifier;
    }

    static E_BehaviorType convertStringToBehaviorType( const std::string& behaviorType )
    {
      if ( behaviorType == "JitterBehavior" ) return E_BehaviorType::E_JitterBehavior;
      if ( behaviorType == "FreeFallBehavior" ) return E_BehaviorType::E_FreeFallBehavior;
      if ( behaviorType == "RadialSpreaderBehavior" ) return E_BehaviorType::E_RadialSpreaderBehavior;
      if ( behaviorType == "ColorMorphBehavior" ) return E_BehaviorType::E_ColorMorphBehavior;
      if ( behaviorType == "MagneticBehavior" ) return E_BehaviorType::E_MagneticBehavior;
      return E_BehaviorType::E_InvalidBehavior;
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
  };

}