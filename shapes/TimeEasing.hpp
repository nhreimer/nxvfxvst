#pragma once

#include "helpers/MathHelper.hpp"

namespace nx
{
  class TimeEasing
  {

    enum E_TimeEasingType : int8_t
    {
      E_Linear,
      E_Quadratic,
      E_Cubic,
      E_Quartic,
      E_Sine,
      E_Expo,
      E_Bounce,
      E_Back
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(E_TimeEasingType,
    {
      { E_Linear, "Linear" },
      { E_Quadratic, "Quadratic" },
      { E_Cubic, "Cubic" },
      { E_Quartic, "Quartic" },
      { E_Sine, "Sine" },
      { E_Expo, "Expo" },
      { E_Bounce, "Bounce" },
      { E_Back, "Back" }
    })

  public:

    nlohmann::json serialize() const
    {
      nlohmann::json json = m_easingType;
      return json;
    }

    void deserialize( const nlohmann::json& j )
    {
      m_easingType = j.template get< E_TimeEasingType >();

      switch ( m_easingType )
      {
        case E_Quadratic: m_easingFunction = easeOutQuad; break;
        case E_Cubic: m_easingFunction = easeOutCubic; break;
        case E_Quartic: m_easingFunction = easeOutQuart; break;
        case E_Sine: m_easingFunction = easeOutSine; break;
        case E_Expo: m_easingFunction = easeOutExpo; break;
        case E_Bounce: m_easingFunction = easeOutBounce; break;
        case E_Back: m_easingFunction = easeOutBack; break;

        // always default to linear
        default: m_easingFunction = easeOutLinear; break;
      }
    }

    void trigger()
    {
      m_lastTriggerTime = m_clock.getElapsedTime().asSeconds();
    }

    [[nodiscard]]
    float getEasing() const
    {
      const float time = std::clamp(
        m_clock.getElapsedTime().asSeconds() / m_decayRate,
        0.f,
        1.f );

      return m_easingFunction( time );
    }

    void drawMenu()
    {
      ImGui::PushID( this );

      ImGui::SliderFloat( "##DecayRate", &m_decayRate, 0.f, 5.f, "Decay Rate %0.2f seconds" );

      if ( drawRadio( "Linear", E_Linear ) ) m_easingFunction = easeOutLinear;
      else if ( drawRadio( "Quadratic", E_Quadratic ) ) m_easingFunction = easeOutQuad;
      else if ( drawRadio( "Cubic", E_Cubic ) ) m_easingFunction = easeOutCubic;
      else if ( drawRadio( "Quartic", E_Quartic ) ) m_easingFunction = easeOutQuart;
      else if ( drawRadio( "Sine", E_Sine ) ) m_easingFunction = easeOutSine;
      else if ( drawRadio( "Expo", E_Expo ) ) m_easingFunction = easeOutExpo;
      else if ( drawRadio( "Bounce", E_Bounce ) ) m_easingFunction = easeOutBounce;
      else if ( drawRadio( "Back", E_Back ) ) m_easingFunction = easeOutBack;

      ImGui::PopID();
    }

  private:

    bool drawRadio( const std::string& label,
                    const E_TimeEasingType easingType )
    {
      if ( ImGui::RadioButton( label.c_str(), easingType == m_easingType ) )
      {
        m_easingType = easingType;
        m_easingFunction = easeOutLinear;

        return true;
      }

      return false;
    }

    static float easeOutLinear( const float t )
    {
      return std::max( 0.f, 1.0f - t );
    }

    // fade out with a curve
    static float easeOutQuad( const float t )
    {
      return 1.f - t * t;
    }

    // slower decay at first, sharp at end
    static float easeOutCubic( const float t )
    {
      return 1.f - ( t * t * t );
    }

    // starts slow, ends very fast
    static float easeOutQuart( const float t )
    {
      const float f = 1.f - t;
      return f * f * f * f;
    }

    // smooth and natural like a wave
    static float easeOutSine( const float t )
    {
      return sinf( t * ( POLY_PI / 2.0f ) );
    }

    // very fast dropoff, fun for spark/burst effect
    static float easeOutExpo( const float t )
    {
      return t == 1.f ? 0.f : 1.f - powf( 2.f, -10.f * t );
    }

    // bounces near the end like jello
    static float easeOutBounce( float t )
    {
      if ( t < 1 / 2.75f )
        return 7.5625f * t * t;

      if ( t < 2 / 2.75f )
      {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
      }

      if ( t < 2.5 / 2.75 )
      {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
      }

      t -= 2.625f / 2.75f;
      return 7.5625f * t * t + 0.984375f;
    }

    // overshoots slightly before settling down
    static float easeOutBack( float t )
    {
      constexpr float c1 = 1.70158f;
      constexpr float c3 = c1 + 1.0f;
      return 1 + c3 * powf( t - 1, 3 ) + c1 * powf( t - 1, 2 );
    }

  private:
    float m_lastTriggerTime { INT32_MIN };
    float m_decayRate { 0.f };
    sf::Clock m_clock;

    E_TimeEasingType m_easingType { E_Linear };
    std::function< float( float t ) > m_easingFunction { easeOutLinear };

  };
}