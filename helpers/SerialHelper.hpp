#pragma once

#include <nlohmann/json.hpp>

#include "models/InterfaceTypes.hpp"

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
          case E_ShaderType::E_PulseShader: return "PulseShader";
          case E_ShaderType::E_RippleShader: return "RippleShader";
          case E_ShaderType::E_StrobeShader: return "StrobeShader";
          case E_ShaderType::E_RumbleShader: return "RumbleShader";
          case E_ShaderType::E_KaleidoscopeShader: return "KaleidoscopeShader";
          case E_ShaderType::E_SmearShader: return "SmearShader";
          case E_ShaderType::E_DensityHeatMapShader: return "DensityHeatMapShader";
          case E_ShaderType::E_FeedbackShader: return "FeedbackShader";
          case E_ShaderType::E_DualKawaseBlurShader: return "DualKawaseBlurShader";
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
        return E_LayoutType::E_EmptyLayout;
      }
      else if constexpr ( std::is_same_v< TEnum, E_ShaderType > )
      {
        if ( typeName == "GlitchShader" ) return E_ShaderType::E_GlitchShader;
        if ( typeName == "PulseShader" ) return E_ShaderType::E_PulseShader;
        if ( typeName == "BlurShader" ) return E_ShaderType::E_BlurShader;
        if ( typeName == "StrobeShader" ) return E_ShaderType::E_StrobeShader;
        if ( typeName == "RippleShader" ) return E_ShaderType::E_RippleShader;
        if ( typeName == "KaleidoscopeShader" ) return E_ShaderType::E_KaleidoscopeShader;
        if ( typeName == "RumbleShader" ) return E_ShaderType::E_RumbleShader;
        if ( typeName == "SmearShader" ) return E_ShaderType::E_SmearShader;
        if ( typeName == "DensityHeatMapShader" ) return E_ShaderType::E_DensityHeatMapShader;
        if ( typeName == "FeedbackShader" ) return E_ShaderType::E_FeedbackShader;
        if ( typeName == "DualKawaseBlurShader" ) return E_ShaderType::E_DualKawaseBlurShader;
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
        if ( typeName == "RadialSpreaderBehavior" ) return E_BehaviorType::E_RadialSpreaderBehavior;
        if ( typeName == "ColorMorphBehavior" ) return E_BehaviorType::E_ColorMorphBehavior;
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
  };

}