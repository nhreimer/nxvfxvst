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
      E_Back,

      E_SmoothPulseDecay,
      E_Impulse,
      E_SparkleFlicker,
      E_PulsePing,
      E_PulseSine
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
      { E_Back, "Back" },
      { E_SmoothPulseDecay, "SmoothPulse" },
      { E_Impulse, "Impulse" },
      { E_SparkleFlicker, "SparkleFlicker" },
      { E_PulsePing, "PulsePing" },
      { E_PulseSine, "PulseSine" }
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
        case E_PulsePing: m_easingFunction = pulsePing; break;
        case E_PulseSine: m_easingFunction = pulseSine; break;
        case E_Impulse: m_easingFunction = impulse; break;
        case E_SparkleFlicker: m_easingFunction = sparkleFlicker; break;
        case E_SmoothPulseDecay: m_easingFunction = smoothDecayPulse; break;

        // always default to linear
        default: m_easingFunction = easeOutLinear; break;
      }
    }

    void trigger()
    {
      m_clock.restart();
    }

    [[nodiscard]]
    float getEasing() const
    {
      switch ( m_easingType )
      {
        // use elapsed over decay by a multiplier
        case E_Impulse:
          return m_easingFunction( m_clock.getElapsedTime().asSeconds() / m_decayRate * m_scale );

        case E_SparkleFlicker:
          return m_easingFunction( m_clock.getElapsedTime().asSeconds() / m_decayRate ) * m_intensity;
        // case E_PulseSine:
        //   return m_easingFunction( m_clock.getElapsedTime().asSeconds() );

        // use elapsed over decay
        // case E_Linear:
        // case E_Quadratic:
        // case E_Cubic:
        // case E_Quartic:
        // case E_Sine:
        // case E_Expo:
        // case E_Bounce:
        // case E_Back:
        // case E_SmoothPulseDecay:
        // case E_PulsePing:
        default:
         return m_easingFunction( std::clamp(
           m_clock.getElapsedTime().asSeconds() / m_decayRate,
           0.f,
           1.f ) );
      }
    }

    void drawMenu()
    {
      ImGui::PushID( this );

      ImGui::SliderFloat( "##DecayRate", &m_decayRate, 0.f, 1.5f, "Decay Rate %0.2f seconds" );
      if ( m_easingType == E_SparkleFlicker )
        ImGui::SliderFloat( "##Sparkle Intensity", &m_intensity, 0.f, 3.f, "Intensity %0.2f" );
      else if ( m_easingType == E_Impulse )
        ImGui::SliderFloat( "##Impulse Scale", &m_scale, 0.f, 10.f, "Scale %0.2f" );

      ImGui::Text( "Time Easing Functions:" );

      if ( drawRadio( "Linear", E_Linear ) ) m_easingFunction = easeOutLinear;
      else if ( drawRadio( "Quadratic", E_Quadratic, true ) ) m_easingFunction = easeOutQuad;
      else if ( drawRadio( "Cubic", E_Cubic, true ) ) m_easingFunction = easeOutCubic;
      else if ( drawRadio( "Quartic", E_Quartic, true ) ) m_easingFunction = easeOutQuart;

      else if ( drawRadio( "Sine", E_Sine ) ) m_easingFunction = easeOutSine;
      else if ( drawRadio( "Expo", E_Expo, true ) ) m_easingFunction = easeOutExpo;
      else if ( drawRadio( "Bounce", E_Bounce, true ) ) m_easingFunction = easeOutBounce;
      else if ( drawRadio( "Back", E_Back, true ) ) m_easingFunction = easeOutBack;
      else if ( drawRadio( "Impulse", E_Impulse, true ) ) m_easingFunction = impulse;

      else if ( drawRadio( "PulseSine", E_PulseSine ) ) m_easingFunction = pulseSine;
      else if ( drawRadio( "PulsePing", E_PulsePing, true ) ) m_easingFunction = pulsePing;
      else if ( drawRadio( "SparkleFlicker", E_SparkleFlicker, true ) ) m_easingFunction = sparkleFlicker;
      else if ( drawRadio( "SmoothPulse", E_SmoothPulseDecay, true ) ) m_easingFunction = smoothDecayPulse;

      ImGui::PopID();
    }

  private:

    bool drawRadio( const std::string& label,
                    const E_TimeEasingType easingType,
                    const bool useSameLine = false )
    {
      if ( useSameLine ) ImGui::SameLine();

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

    // Pulses up and down rhythmically — smooth, continuous wave.
    // use TOTAL time not elapsed for constant pulsating
    static float pulseSine( const float t )
    {
      return 0.5f * ( 1.0f + sinf( t * 2.0f * POLY_PI ) ); // cycles every 1 second
    }

    // Starts strong, then rings down in pulses like an echo.
    // Use with t = elapsed / decayRate
    static float pulsePing( const float t )
    {
      const float ring = sinf(t * 20.f);         // 10Hz wave
      const float decay = expf(-3.f * t);        // fast exponential decay
      return std::max( 0.f, ring * decay ); // clip negatives
    }

    // Feed it totalTime — not tied to trigger.
    // You can scale it by intensity to make sparkles appear only when active.
    static float sparkleFlicker( const float t )
    {
      return fmodf(sinf(t * 123.456f) * 43758.5453f, 1.0f); // hashy sparkle
    }

    // Like a heartbeat or energy blast with a long tail.
    // Use t = elapsed / decayRate, but maybe scale t like t *= 3.0f to shift the peak early.
    static float impulse( const float t )
    {
      return t * expf(1.0f - t);
    }

    // Still pulses, but smoothly fades with time.
    static float smoothDecayPulse( const float t )
    {
      return sinf(12.f * t) * expf(-4.0f * t);
    }

  private:
    float m_decayRate { 1.f };
    float m_intensity { 0.f }; // used for certain ones
    float m_scale { 3.f }; // used for certain ones

    sf::Clock m_clock;

    E_TimeEasingType m_easingType { E_Linear };
    std::function< float( float t ) > m_easingFunction { easeOutLinear };

  };
}