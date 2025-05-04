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