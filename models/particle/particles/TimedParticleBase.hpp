#pragma once

#include "models/IParticle.hpp"

namespace nx
{

  class TimedParticleBase : public IParticle
  {
  public:

    float getTimeRemainingPercentage() const override
    {
      return m_timeAliveInSeconds / ( m_expirationTimeInSeconds - m_spawnTimeInSeconds );
    }

    bool hasExpired() const override { return getTimeRemainingPercentage() >= 1.0f; }

    float getTimeAliveInSeconds() const override { return m_timeAliveInSeconds; }

    void update( const sf::Time& deltaTime ) override
    {
      m_timeAliveInSeconds += deltaTime.asSeconds();
    }

    float getSpawnTimeInSeconds() const override
    {
      return m_spawnTimeInSeconds;
    }

    void setSpawnTimeInSeconds(const float newSpawnTime) override
    {
      m_spawnTimeInSeconds = newSpawnTime;
    }

    float getExpirationTimeInSeconds() const override
    {
      return m_expirationTimeInSeconds;
    }

    void setExpirationTimeInSeconds(const float expirationTime) override
    {
      m_expirationTimeInSeconds = expirationTime;
    }

  protected:

    float m_spawnTimeInSeconds { 0.f };
    float m_expirationTimeInSeconds { 0.f };
    float m_timeAliveInSeconds { 0.f };
  };


}