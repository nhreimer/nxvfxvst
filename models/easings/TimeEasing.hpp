#pragma once

#include "models/easings/EasingsBase.hpp"

namespace nx
{

  class TimeEasing final : public EasingsBase
  {

  public:

    void trigger()
    {
      // when using E_Time, do NOT reset the clock or else you'll get a linear-style effect
      m_timeTriggeredInSeconds = ( m_data.easingType == E_EasingType::E_TimeContinuous )
        ? m_clock.getElapsedTime().asSeconds()
        : m_clock.restart().asSeconds();
    }

    float getElapsedTime() const
    {
      return m_clock.getElapsedTime().asSeconds();
    }

    [[nodiscard]]
    float getEasing() const
    {

      const float decay = m_data.decayRate <= 0.001f ? 0.f : m_clock.getElapsedTime().asSeconds() / m_data.decayRate;

      switch ( m_data.easingType )
      {
        case E_EasingType::E_Disabled:
          return 1.f;

        case E_EasingType::E_Fixed:
          return m_data.intensity;

        case E_EasingType::E_TimeContinuous:
        case E_EasingType::E_TimeIntervallic:
          return m_clock.getElapsedTime().asSeconds();

          // use elapsed over decay by a multiplier
        case E_EasingType::E_Impulse:
          return m_easingFunction( decay * m_data.intensity );

        case E_EasingType::E_SparkleFlicker:
          return m_easingFunction( decay ) * m_data.intensity;

        default:
          return m_easingFunction( std::clamp(
            decay,
            0.f,
            1.f ) );
      }
    }

  private:

    float m_timeTriggeredInSeconds { 0.f };
    sf::Clock m_clock;


  };
}