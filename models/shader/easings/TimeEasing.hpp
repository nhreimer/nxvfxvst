#pragma once

#include "helpers/MathHelper.hpp"

namespace nx
{

  enum E_TimeEasingType : int8_t
  {
    E_None,
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

  struct TimeEasingData_t
  {
    float decayRate { 0.1f };
    float intensity { 0.f }; // used for certain ones

    E_TimeEasingType easingType { E_None };
  };

  class TimeEasing
  {

  public:

    nlohmann::json serialize() const
    {
      nlohmann::json json = m_data.easingType;
      return json;
    }

    void deserialize( const nlohmann::json& j )
    {
      m_data.easingType = j.get< E_TimeEasingType >();
      setEasingFunction( m_data.easingType );
    }

    float getDecayRate() const { return m_data.decayRate; }

    void trigger()
    {
      m_timeTriggeredInSeconds = m_clock.restart().asSeconds();
    }

    float getElapsedTime() const { return m_clock.getElapsedTime().asSeconds(); }

    TimeEasingData_t& getData() { return m_data; }
    void setData( const TimeEasingData_t& data )
    {
      m_data = data;
      setEasingFunction( m_data.easingType );
    }

    [[nodiscard]]
    float getEasing() const
    {
      const float decay = m_data.decayRate <= 0.001f ? 0.f : m_clock.getElapsedTime().asSeconds() / m_data.decayRate;

      switch ( m_data.easingType )
      {
        case E_None:
          return m_clock.getElapsedTime().asSeconds();

        // use elapsed over decay by a multiplier
        case E_Impulse:
          return m_easingFunction( decay * m_data.intensity );

        case E_SparkleFlicker:
          return m_easingFunction( decay ) * m_data.intensity;

        default:
         return m_easingFunction( std::clamp(
           decay,
           0.f,
           1.f ) );
      }
    }

    void drawMenu()
    {
      ImGui::PushID( this );

      ImGui::SliderFloat( "##DecayRate", &m_data.decayRate, 0.01f, 1.5f, "Decay Rate %0.2f seconds" );
      if ( m_data.easingType == E_SparkleFlicker )
        ImGui::SliderFloat( "##Sparkle Intensity", &m_data.intensity, 0.f, 3.f, "Intensity %0.2f" );
      else if ( m_data.easingType == E_Impulse )
        ImGui::SliderFloat( "##Impulse Scale", &m_data.intensity, 0.f, 10.f, "Scale %0.2f" );

      ImGui::Text( "Time Easing Functions:" );

      if ( drawRadio( "None", E_None ) ) m_easingFunction = useNone;
      else if ( drawRadio( "Linear", E_Linear ) ) m_easingFunction = easeOutLinear;
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

      if ( ImGui::RadioButton( label.c_str(), easingType == m_data.easingType ) )
      {
        m_data.easingType = easingType;
        return true;
      }

      return false;
    }

    void setEasingFunction( const E_TimeEasingType easingType )
    {
      switch ( easingType )
      {
        case E_None: m_easingFunction = useNone; break;
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
        default:
          m_easingFunction = easeOutLinear; break;
      }
    }

    static float useNone( const float t )
    {
      return t;
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
      return sinf( t * ( NX_PI / 2.0f ) );
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
      return 0.5f * ( 1.0f + sinf( t * 2.0f * NX_PI ) ); // cycles every 1 second
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

    float m_timeTriggeredInSeconds { 0.f };
    TimeEasingData_t m_data;
    sf::Clock m_clock;
    std::function< float( float t ) > m_easingFunction { useNone };

    NLOHMANN_JSON_SERIALIZE_ENUM(E_TimeEasingType,
    {
      { E_None, "None" },
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

  };
}