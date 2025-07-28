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

#include <fstream>
#include <nlohmann/json.hpp>

#include "models/data/Midi_t.hpp"

namespace nx
{
  // struct EventRecord_t
  // {
  //   int64_t frameNumber { 0 };
  //   float timeSeconds { 0.f };
  //   int16_t channel { 0 };
  //   int16_t pitch { 0 };
  // };

  class EventRecorder final
  {
    public:

      EventRecorder()
        : m_journal( nlohmann::json::array() )
      {}

      void clear()
      {
        m_journal.clear();
      }

      void addEvent( const int64_t frameCount, const Midi_t& midiEvent, const float timeInSeconds )
      {
        const auto it = m_channelEvents.find( midiEvent.channel );

        if ( it == m_channelEvents.end() )
        {
          m_journal.push_back(
            {
              { "frameNumber", frameCount },
              { "timeInSeconds", timeInSeconds },
              { "channel", midiEvent.channel },
              { "pitch", midiEvent.pitch }
            });

          m_channelEvents.emplace( midiEvent.channel, 1 );
        }
        else
          it->second++;
      }

      void saveToFile( const std::string& fileName ) const
      {
        std::ofstream file( fileName );
        file << m_journal;
        file.close();
      }

    private:

      nlohmann::json m_journal;
      std::unordered_map< int16_t, int32_t > m_channelEvents;
  };
}