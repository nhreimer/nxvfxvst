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
    static std::tuple< int32_t , int32_t > getMidiNote( int32_t noteValue )
    {
      int32_t deoffset = noteValue - MIDI_FLOOR;
      int32_t noteName = ( deoffset % 12 );
      auto octave = static_cast< int32_t >( static_cast< float >( deoffset ) / 12.f );

      return { noteName, octave };
    }

    inline static int32_t convertToMidiNote( int32_t midiNote, int32_t midiOctave )
    {
      return midiOctave * 12 + MIDI_FLOOR + midiNote;
    }

  private:

    inline static const int MIDI_FLOOR = 21;

  };

}