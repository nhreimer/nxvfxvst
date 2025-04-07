#pragma once

#include "shapes/TimedCursorPosition.hpp"
#include "shapes/MidiNoteControl.hpp"

namespace nx
{
  struct RippleData_t
  {
    bool isActive { true };

    float rippleCenterX { 0.5f };
    float rippleCenterY { 0.5f };

    float amplitude { 0.05f };
    float frequency { 10.0f };
    float speed { 0.f };
    float pulseDecay { -1.0f };
  };

  class RippleShader final : public IShader
  {
  public:

    explicit RippleShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load ripple fragment shader" );
      }
    }

    ~RippleShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", SerialHelper::convertShaderTypeToString( getType() ) },
          { "isActive", m_data.isActive },
          { "rippleCenterX", m_data.rippleCenterX },
          { "rippleCenterY", m_data.rippleCenterY },
          { "amplitude", m_data.amplitude },
          { "frequency", m_data.frequency },
          { "speed", m_data.speed },
          { "pulseDecay", m_data.pulseDecay }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.rippleCenterX = j.value("rippleCenterX", 0.5f);
      m_data.rippleCenterY = j.value("rippleCenterY", 0.5f);
      m_data.amplitude = j.value("amplitude", 0.05f);
      m_data.frequency = j.value("frequency", 10.0f);
      m_data.speed = j.value("speed", 0.f);
      m_data.pulseDecay = j.value("pulseDecay", -1.0f);
    }

    E_ShaderType getType() const override { return E_RippleShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Ripple" ) )
      {
        ImGui::Checkbox( "Ripple Active##1", &m_data.isActive );

        if ( ImGui::SliderFloat( "Ripple Center x##1", &m_data.rippleCenterX, -1.f, 1.f ) ||
             ImGui::SliderFloat( "Ripple Center y##1", &m_data.rippleCenterY, -1.f, 1.f ) )
        {
          const sf::Vector2f calibrated { ( m_data.rippleCenterX + 1.f ) / 2.f * m_globalInfo.windowSize.x,
                                          ( m_data.rippleCenterY + 1.f ) / 2.f * m_globalInfo.windowSize.y };
          m_timedCursor.setPosition( calibrated );
        }

        // ImGui::SliderFloat( "Ripple Amplitude##1", &m_data.amplitude, 0.f, 0.05f );
        ImGui::SliderFloat( "Ripple Frequency##1", &m_data.frequency, 10.f, 50.f );
        ImGui::SliderFloat( "Ripple Speed##1", &m_data.speed, 0.f, 10.f );
        ImGui::SliderFloat( "Ripple Pulse Decay##1", &m_data.pulseDecay, -2.f, 0.f );

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_lastTriggerTime = m_clock.getElapsedTime().asSeconds();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.amplitude > 0.f; }

    [[nodiscard]]
    sf::RenderTexture & applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize ripple texture" );
        }
      }

      const float time = m_clock.getElapsedTime().asSeconds();
      const float timeSinceTrigger = time - m_lastTriggerTime;

      // Pulse decay: fast pulse that fades over ~1 second
      float pulse = exp( m_data.pulseDecay * timeSinceTrigger );  // decays quickly
      pulse = std::clamp( pulse, 0.0f, 1.0f ); // just in case

      constexpr float baseAmplitude = 0.005f;
      constexpr float maxPulseAmplitude = 0.03f;

      m_data.amplitude = baseAmplitude + pulse * maxPulseAmplitude;

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform( "resolution", sf::Vector2f( inputTexture.getSize() ) );
      m_shader.setUniform( "time", time );

      m_shader.setUniform( "rippleCenter", sf::Vector2f( m_data.rippleCenterX, m_data.rippleCenterY) );
      m_shader.setUniform( "amplitude", m_data.amplitude );     // 0.0f – 0.05f
      m_shader.setUniform( "frequency", m_data.frequency );     // 10.0f – 50.0f
      m_shader.setUniform( "speed", m_data.speed );             // 0.0f – 10.0f

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    RippleData_t m_data;

    float m_lastTriggerTime { INT32_MIN };

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    TimedCursorPosition m_timedCursor;
    MidiNoteControl m_midiNoteControl;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;

uniform vec2 rippleCenter;
uniform float amplitude;
uniform float frequency;
uniform float speed;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    // Offset from ripple center (in UV space)
    vec2 delta = uv - rippleCenter;
    float dist = length(delta);

    // Ripple offset
    float ripple = sin(dist * frequency - time * speed) * amplitude;

    // Normalize direction
    vec2 dir = normalize(delta);
    uv += dir * ripple;

    gl_FragColor = texture2D(texture, uv);
})";
  };

}