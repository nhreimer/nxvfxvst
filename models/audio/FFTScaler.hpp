#pragma once

namespace nx
{

  class FFTScaler final
  {
    enum class Mode
    {
      Linear,
      Decibel
    };

    struct FFTScalerData_t
    {
      Mode mode = Mode::Decibel;
      float gain = 1.f;
      float dbFloor = -60.f;
      float curvePower = 0.25f;
      bool clamp = true;
    };

  public:

    void scale( AudioDataBuffer& bins ) const
    {
      for ( auto& val : bins )
      {
        float v = std::max( val * m_data.gain, 1e-8f ); // Prevent log(0)

        if ( m_data.mode == Mode::Decibel )
        {
          const float db = 20.f * std::log10( v );
          v = ( db - m_data.dbFloor ) / std::abs( m_data.dbFloor ); // normalize to 0..1
        }

        // Optional shaping (e.g. sqrt, pow(0.5), pow(0.3) etc)
        v = std::pow( v, m_data.curvePower );

        // Optional clamping
        if ( m_data.clamp )
          v = std::clamp( v, 0.f, 1.f );

        val = v;
      }
    }

    void drawMenu()
    {
      if ( ImGui::TreeNode( "FFT Scaler" ) )
      {
        if ( ImGui::RadioButton( "Linear", m_data.mode == Mode::Linear ) )
          m_data.mode = Mode::Linear;
        else if ( ImGui::RadioButton( "Decibel", m_data.mode == Mode::Decibel ) )
          m_data.mode = Mode::Decibel;

        ImGui::SliderFloat("Gain##1", &m_data.gain, 0.1f, 10.f, "%.2fx");
        ImGui::SliderFloat("dB Floor##1", &m_data.dbFloor, -100.f, -20.f);
        ImGui::SliderFloat("Curve Power", &m_data.curvePower, 0.1f, 1.5f, "%.2f");

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:
    FFTScalerData_t m_data;
  };

}