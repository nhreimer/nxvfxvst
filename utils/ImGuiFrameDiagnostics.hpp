#pragma once

#ifdef WIN32
#include "vst/views/Win32Helper.hpp"
#endif

namespace nx
{

  inline double getRefreshRateHz()
  {
#ifdef _WIN32
    return win32::Win32Helper::getRefreshRateHz();
#else
    return 60.0;
#endif
  }

  struct ImGuiFrameDiagnostics
  {
    float currentFps = 0.f;
    float smoothedFps = 0.f;
    float currentDeltaMs = 0.f;

    double vsyncRateHz = 60.f;

    void update(const sf::Time& deltaTime)
    {
      currentDeltaMs = deltaTime.asSeconds() * 1000.f;
      currentFps = (deltaTime.asSeconds() > 0.0f)
                       ? 1.0f / deltaTime.asSeconds()
                       : 0.f;

      // Apply exponential smoothing
      smoothedFps = 0.9f * smoothedFps + 0.1f * currentFps;

      m_lastRefreshTime += deltaTime.asSeconds();
      if ( m_lastRefreshTime >= m_maxVSyncRefreshWaitInSecs )
      {
        vsyncRateHz = getRefreshRateHz();
        m_lastRefreshTime = 0.f;
      }
    }

    void drawMenu() const
    {
      const auto expectedFps = static_cast<float>(vsyncRateHz);
      const bool droppingFrames = smoothedFps < expectedFps * (1.f - m_tolerance);

      ImGui::SeparatorText( "Frame Rate Metrics" );

      // Flash red if frames are dropping
      if (droppingFrames)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 60, 60, 255));
      else
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 255, 200, 255));

      ImGui::Text("Delta:   %.2f ms", currentDeltaMs);
      ImGui::Text("FPS:     %.2f (smoothed)", smoothedFps);
      ImGui::Text("VSync:   %.2f Hz", vsyncRateHz);

      ImGui::PopStyleColor();
    }

  private:

    // Determine FPS health
    static constexpr float m_tolerance = 0.15f; // Allow 15% frame drift
    static constexpr float m_maxVSyncRefreshWaitInSecs = 10.f;
    float m_lastRefreshTime = 10.f;
  };

}