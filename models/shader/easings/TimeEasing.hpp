#pragma once

#include "helpers/MathHelper.hpp"

namespace nx
{

  enum class E_TimeEasingType : int8_t
  {
    E_Disabled,        // bypass completely (always returns 1)
    E_Fixed,           // fixed number: uses intensity only
    E_TimeContinuous,  // elapsed time
    E_TimeIntervallic, // time resets
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
    E_PulseSine,
    E_Elastic
  };

  class TimeEasing
  {

     struct TimeEasingData_t
    {
      float decayRate { 0.1f };
      float intensity { 0.f }; // used for certain ones

      E_TimeEasingType easingType { E_TimeEasingType::E_Linear };
    };

  public:

    nlohmann::json serialize() const;

    void deserialize( const nlohmann::json& j );

    E_TimeEasingType getEasingType() const { return m_data.easingType; }

    void setEasingType( const E_TimeEasingType easingType );

    float getDecayRate() const;

    void trigger();

    float getElapsedTime() const;

    TimeEasingData_t& getData();

    void setData( const TimeEasingData_t& data );

    [[nodiscard]]
    float getEasing() const;

    void drawMenu();

  private:

    void setEasingFunction( const E_TimeEasingType easingType );

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

    static float easeOutElastic(float t)
    {
      constexpr float c4 = NX_TAU / 3.f; //(2.0f * 3.14159265f) / 3.0f;

      if (t == 0.0f)
        return 0.0f;
      if (t == 1.0f)
        return 1.0f;

      return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }

  private:

    float m_timeTriggeredInSeconds { 0.f };
    TimeEasingData_t m_data;
    sf::Clock m_clock;
    std::function< float( float t ) > m_easingFunction { useNone };

    int32_t m_selectedComboIndex { 4 };

    inline static std::array< const char *, 18 > m_easingFunctionNames =
    {
      "Disabled",
      "Fixed",
      "TimeContinuous",
      "TimeIntervallic",
      "Linear",
      "Quadratic",
      "Cubic",
      "Quartic",
      "Sine",
      "Expo",
      "Bounce",
      "Back",
      "SmoothPulse",
      "Impulse",
      "SparkleFlicker",
      "PulsePing",
      "PulseSine",
      "Elastic"
    };

    inline static std::array< E_TimeEasingType, 18 > m_easingTypes =
    {
      E_TimeEasingType::E_Disabled,
      E_TimeEasingType::E_Fixed,
      E_TimeEasingType::E_TimeContinuous,
      E_TimeEasingType::E_TimeIntervallic,
      E_TimeEasingType::E_Linear,
      E_TimeEasingType::E_Quadratic,
      E_TimeEasingType::E_Cubic,
      E_TimeEasingType::E_Quartic,
      E_TimeEasingType::E_Sine,
      E_TimeEasingType::E_Expo,
      E_TimeEasingType::E_Bounce,
      E_TimeEasingType::E_Back,
      E_TimeEasingType::E_SmoothPulseDecay,
      E_TimeEasingType::E_Impulse,
      E_TimeEasingType::E_SparkleFlicker,
      E_TimeEasingType::E_PulsePing,
      E_TimeEasingType::E_PulseSine,
      E_TimeEasingType::E_Elastic
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(E_TimeEasingType,
    {
      { E_TimeEasingType::E_Disabled, "Disabled" },
      { E_TimeEasingType::E_Fixed, "Fixed" },
      { E_TimeEasingType::E_TimeContinuous, "TimeContinuous" },
      { E_TimeEasingType::E_TimeIntervallic, "TimeIntervallic" },
      { E_TimeEasingType::E_Linear, "Linear" },
      { E_TimeEasingType::E_Quadratic, "Quadratic" },
      { E_TimeEasingType::E_Cubic, "Cubic" },
      { E_TimeEasingType::E_Quartic, "Quartic" },
      { E_TimeEasingType::E_Sine, "Sine" },
      { E_TimeEasingType::E_Expo, "Expo" },
      { E_TimeEasingType::E_Bounce, "Bounce" },
      { E_TimeEasingType::E_Back, "Back" },
      { E_TimeEasingType::E_SmoothPulseDecay, "SmoothPulse" },
      { E_TimeEasingType::E_Impulse, "Impulse" },
      { E_TimeEasingType::E_SparkleFlicker, "SparkleFlicker" },
      { E_TimeEasingType::E_PulsePing, "PulsePing" },
      { E_TimeEasingType::E_PulseSine, "PulseSine" },
      { E_TimeEasingType::E_Elastic, "Elastic" }
    })

  };
}