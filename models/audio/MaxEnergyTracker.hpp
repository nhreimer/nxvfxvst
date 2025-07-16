#pragma once

namespace nx
{
  class MaxEnergyTracker
  {
  public:
    float updateMaxEnergy(const float mag)
    {
      // Smooth decay and track new spikes
      m_recentMax = std::lerp(m_recentMax, mag, 0.05f);
      if (mag > m_recentMax)
        m_recentMax = mag;

      return m_recentMax;
    }

    [[nodiscard]]
    float getRecentMaxEnergy() const { return m_recentMax; }

    void resetMaxEnergy() { m_recentMax = 0.1f; }

  private:

    float m_recentMax { 0.1f };
  };
}