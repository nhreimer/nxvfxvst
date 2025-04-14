#pragma once

namespace nx
{

  struct LissajousCurveLayoutData_t : public ParticleLayoutData_t
  {
    float phaseAStep { 2.0f };
    float phaseBStep { 3.0f };

    float phaseDelta { 0.5f };
    float phaseSpread { 0.5f };

    // uint8_t tracerCount = 8;
    // float tracerDelayStep = 0.04f; // how far apart each ghost is in phase
    // float fadeExponent = 1.5f;     // higher = faster fade
    // float sizeFalloff = 0.75f;     // scaling down radius
  };

  /// This layout places each particle along a mathematically beautiful Lissajous curve using parametric sine functions
  /// x = A * sin(a * t + δ)
  /// y = B * sin(b * t)
  /// a = pitch-based frequency
  /// b = velocity-based frequency
  /// t = time or phase (based on global time)
  /// δ = offset to prevent overlap or to animate drift
  class LissajousCurveLayout final : public ParticleConsumer< LissajousCurveLayoutData_t >
  {
  public:

    explicit LissajousCurveLayout( const GlobalInfo_t& globalInfo )
      : ParticleConsumer( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LissajousCurveLayout; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Lissajous Curve Layout" ) )
      {
        ParticleHelper::drawMenu( m_data );

        //ImGui::SliderFloat("Phase", &m_data.phase, 0.f, 1.0f);
        ImGui::SliderFloat("a Phase Step", &m_data.phaseAStep, 0.f, 5.0f);
        ImGui::SliderFloat("b Phase Step", &m_data.phaseBStep, 0.f, 5.0f);
        ImGui::SliderFloat("Phase Delta", &m_data.phaseDelta, 0.f, 1.0f);
        ImGui::SliderFloat("Phase Spread", &m_data.phaseSpread, 0.f, 1.0f);

        ImGui::TreePop();
        ImGui::Separator();
      }
    }

  protected:
    sf::Vector2f getNextPosition( const Midi_t & midiEvent ) override
    {
      const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
      const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency
      const float t = m_globalInfo.elapsedTimeSeconds; //m_data.phase;

      const float x = ( m_globalInfo.windowHalfSize.x * m_data.phaseSpread ) * sin(a * t + m_data.phaseDelta);
      const float y = ( m_globalInfo.windowHalfSize.y * m_data.phaseSpread ) * sin(b * t);

      return { x, y };
    }

  private:

  };
}