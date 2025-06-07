#pragma once

#include "helpers/MathHelper.hpp"

namespace nx
{

  class Easings
  {
  public:
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
    static float easeOutBack( const float t )
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

    static float easeOutElastic(const float t)
    {
      constexpr float c4 = NX_TAU / 3.f; //(2.0f * 3.14159265f) / 3.0f;

      if (t == 0.0f)
        return 0.0f;
      if (t == 1.0f)
        return 1.0f;

      return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
  };
}