#pragma once

#include <SFML/System/Clock.hpp>


#include "models/easings/EasingsBase.hpp"

namespace nx
{

  class PercentageEasing final : public EasingsBase
  {
  public:
    [[nodiscard]]
    float getEasing( const float t ) const
    {
      const float decay = m_data.decayRate <= 0.001f ? 0.f : m_data.decayRate;

      switch ( m_data.easingType )
      {
        case E_EasingType::E_Disabled:
          return 1.f;

        case E_EasingType::E_Fixed:
          return m_data.intensity;

        case E_EasingType::E_TimeContinuous:
        case E_EasingType::E_TimeIntervallic:
          return Easings::easeOutLinear( t );

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
    sf::Clock m_clock;
  };

}