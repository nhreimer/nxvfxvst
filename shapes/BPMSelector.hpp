/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

namespace nx
{
  class BpmDivisionSelector
  {
  public:

    explicit BpmDivisionSelector( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    void drawMenu()
    {
      if (ImGui::BeginCombo("BPM Selector", divisions[ m_selected ].first))
      {
        for (int i = 0; i < divisions.size(); ++i)
        {
          const bool is_selected = (m_selected == i);
          if (ImGui::Selectable(divisions[ i ].first, is_selected))
          {
            m_selected = i;
            update();
          }
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::Text("Duration: %s (%.2f/sec)", m_currentLabel, m_currentDuration);

    }

    float getDurationSeconds() const { return m_currentDuration; }
    const char *getLabel() const { return m_currentLabel; }

  private:

    void update()
    {
      m_currentLabel = divisions[ m_selected ].first;
      m_currentDuration = (60.f / m_globalInfo.bpm) * divisions[ m_selected ].second;
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    int32_t m_selected { 0 };

    float m_currentDuration { 0 };
    const char * m_currentLabel { nullptr };
    // float bpm = 120.f;
    // const char * currentLabel = "1/4";
    // float currentDuration = 0.5f; // seconds

    inline static const std::vector< std::pair< const char *, float > > divisions =
    {
      { "1/1", 4.f },    { "1/2", 2.f },        { "1/2.", 3.f },       { "1/2T", 4.f / 3.f },
      { "1/4", 1.f },    { "1/4.", 1.5f },      { "1/4T", 2.f / 3.f }, { "1/8", 0.5f },
      { "1/8.", 0.75f }, { "1/8T", 1.f / 3.f }, { "1/16", 0.25f },     { "1/16T", 1.f / 6.f }
    };
  };

} // namespace nx
