#pragma once

namespace nx
{
  class MidiNoteControl
  {
  public:

    nlohmann::json serialize() const
    {
      nlohmann::json j = m_midiNotes;
      return j;
    }

    void deserialize( const nlohmann::json & j )
    {
      m_midiNotes = j.get< std::vector< int16_t > >();
    }

    void drawMenu()
    {
      ImGui::Text("Add MIDI Note:");

      ImGui::Combo("Note", &selectedNoteIndex, m_kNoteNames, IM_ARRAYSIZE(m_kNoteNames), ImGuiComboFlags_WidthFitPreview);
      ImGui::InputInt("Octave", &selectedNoteOctave);

      selectedNoteOctave = std::clamp(selectedNoteOctave, -1, 9);  // Reasonable octave range

      if ( ImGui::Button( "Add Note" ) )
      {
        const int midiNote = ( selectedNoteOctave + 1 ) * 12 + selectedNoteIndex;
        if ( midiNote >= 0 && midiNote <= 127 )
        {
          // prevent duplicates
          if ( std::ranges::find(m_midiNotes, midiNote ) == m_midiNotes.end() )
            m_midiNotes.push_back( static_cast< int16_t >( midiNote ) );
        }
      }

      ImGui::Separator();
      ImGui::Text( "Active Triggers:" );
      for ( int i = 0; i < m_midiNotes.size(); ++i )
      {
        const int note = m_midiNotes[ i ];
        const int noteIndex = note % 12;
        const int octave = ( note / 12 ) - 1;

        ImGui::Text("%s%d (%d)", m_kNoteNames[ noteIndex ], octave, note );
        ImGui::SameLine();
        if (ImGui::SmallButton(( "X##" + std::to_string( i ) ).c_str() ) )
        {
          m_midiNotes.erase( m_midiNotes.begin() + i );
          --i;
        }
      }
    }

    [[nodiscard]]
    bool empty() const { return m_midiNotes.empty(); }

    [[nodiscard]]
    bool isNoteActive( const int16_t pitch ) const
    {
      return std::ranges::find( m_midiNotes, pitch ) != m_midiNotes.end();
    }

  private:
    int selectedNoteIndex { 0 };
    int selectedNoteOctave { 4 };
    std::vector< int16_t > m_midiNotes;

    static inline const char* m_kNoteNames[ 12 ] =
    {
      "C", "C#", "D", "D#", "E", "F",
      "F#", "G", "G#", "A", "A#", "B"
    };

  };
}