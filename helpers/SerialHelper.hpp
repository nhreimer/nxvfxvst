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
      if constexpr ( std::is_same_v< Enum, E_ShaderType > )
      {
        if ( e == E_EmptyLayout ) return "EmptyLayout";
        if ( e == E_RandomLayout ) return "RandomLayout";
        if ( e == E_SpiralLayout ) return "SpiralLayout";
        if ( e == E_OrbitRingLayout ) return "OrbitRingLayout";
      }
      else if constexpr ( std::is_same_v< Enum, E_ModifierType > )
      {
        if ( e == E_SequentialModifier ) return "SequentialModifier";
        if ( e == E_FullMeshModifier ) return "FullMeshModifier";
        if ( e == E_FreeFallModifier ) return "FreeFallModifier";
        if ( e == E_PerlinDeformerModifier ) return "PerlinDeformerModifier";
      }

      else if constexpr ( std::is_same_v< Enum, E_ShaderType > )
      {
        switch ( e )
        {
          case E_GlitchShader: return "GlitchShader";
          case E_BlurShader: return "BlurShader";
          case E_PulseShader: return "PulseShader";
          case E_RippleShader: return "RippleShader";
          case E_StrobeShader: return "StrobeShader";
          case E_RumbleShader: return "RumbleShader";
          case E_KaleidoscopeShader: return "KaleidoscopeShader";
          case E_SmearShader: return "SmearShader";
          default: break;
        }
      }

      return "UNKNOWN";
    }

    static std::string convertShaderTypeToString( const E_ShaderType shaderType )
    {
      switch ( shaderType )
      {
        case E_ShaderType::E_GlitchShader: return "GlitchShader";
        case E_ShaderType::E_PulseShader: return "PulseShader";
        case E_ShaderType::E_KaleidoscopeShader: return "KaleidoscopeShader";
        case E_ShaderType::E_BlurShader: return "BlurShader";
        case E_ShaderType::E_StrobeShader: return "StrobeShader";
        case E_ShaderType::E_RippleShader: return "RippleShader";
        default: return "";
      }
    }

    static E_ShaderType convertStringToShaderType( const std::string& shaderType )
    {
      if ( shaderType == "GlitchShader" ) return E_GlitchShader;
      if ( shaderType == "PulseShader" ) return E_PulseShader;
      if ( shaderType == "BlurShader" ) return E_BlurShader;
      if ( shaderType == "StrobeShader" ) return E_StrobeShader;
      if ( shaderType == "RippleShader" ) return E_RippleShader;
      if ( shaderType == "KaleidoscopeShader" ) return E_KaleidoscopeShader;
      return E_InvalidShader;
    }

    static E_ModifierType convertStringToModifierType( const std::string& modifierType )
    {
      if ( modifierType == "SequentialModifier" ) return E_SequentialModifier;
      if ( modifierType == "FullMeshModifier" ) return E_FullMeshModifier;
      if ( modifierType == "FreeFallModifier" ) return E_FreeFallModifier;
      if ( modifierType == "PerlinDeformerModifier" ) return E_PerlinDeformerModifier;
      return E_InvalidModifier;
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