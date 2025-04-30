#pragma once

#include <tuple>

namespace nx
{

  struct MidiHelper
  {

    static inline const char * NOTES[] =
    {
      "A",
      "A#/Bb",
      "B",
      "C",
      "C#/Db",
      "D",
      "D#/Eb",
      "E",
      "F",
      "F#/Gb",
      "G",
      "G#/Ab"
      };

    /***
     * Converts a midi integer to its note and octave
     * @param noteValue
     * @return Tuple< noteNumber, noteOctave >
     */
    static std::tuple< int32_t , int32_t > getMidiNote( const int32_t noteValue )
    {
      const auto deoffset = noteValue - MIDI_FLOOR;
      const auto noteName = ( deoffset % 12 );
      const auto octave = static_cast< int32_t >( static_cast< float >( deoffset ) / 12.f );

      return { noteName, octave };
    }

    static constexpr auto convertToMidiNote( const int32_t midiNote, const int32_t midiOctave )
    {
      return midiOctave * 12 + MIDI_FLOOR + midiNote;
    }

  private:

    static constexpr int MIDI_FLOOR = 21;

  };

}