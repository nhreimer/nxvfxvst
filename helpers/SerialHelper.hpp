#pragma once

namespace nx
{

  class SerialHelper
  {
  public:
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