#pragma once

#include "models/shader/easings/TimeEasing.hpp"

namespace nx
{

  class CumulativeEasing
  {
  public:

    void deserialize( const nlohmann::json& j )
    {
      m_masterEasing.deserialize( j.at( "easing" ) );
    }

    nlohmann::json serialize() const
    {
      return m_masterEasing.serialize();
    }

    void drawMenu()
    {
      m_masterEasing.drawMenu();
    }

    void triggerNewBurst()
    {
      auto& burst = m_bursts.emplace_back();
      burst.setData( m_masterEasing.getData() );
      burst.trigger();
      m_lastTriggerInSeconds = m_clock.restart().asSeconds();
    }

    void update( float /*dt*/ )
    {
      std::erase_if( m_bursts,
      []( const TimeEasing& burst )
      {
        return burst.getEasing() <= 0.001f;
      });
    }

    float getCumulativeEasing() const
    {
      float total = 0.f;
      for ( const auto& b : m_bursts )
        total += b.getEasing();

      // prevent nuclear meltdown
      return std::clamp( total, 0.f, 1.5f );
    }

    float getLastTriggeredInSeconds() const { return m_lastTriggerInSeconds; }

  private:

    float m_lastTriggerInSeconds { 0.f };
    sf::Clock m_clock;
    TimeEasing m_masterEasing;
    std::vector< TimeEasing > m_bursts;
  };
}