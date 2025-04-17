#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  struct KaleidoscopeData_t
  {
    bool isActive { true };

    // Slices | Vibe
    //--------|--------------------------
    // 3–4    | Triangular shard symmetry
    // 6–8    | Mandala snowflake bloom
    // 12+    | Fractal kaleido insanity

    float masterGain { 0.1f };
    float slices { 6.f };
    float swirlStrength { 1.f };
    float swirlDensity { 1.f };

    float pulseStrength { 0.2f };
    float pulseFrequency { 10.f };
    float pulseSpeed { 5.f };

    float angleSteps { 32.f };
    float radialStretch { 1.f };
    float noiseStrength { 0.5f };
  };

  class KaleidoscopeShader final : public IShader
  {

    const static inline std::string m_typeName = "KaleidoscopeShader";

  public:

    explicit KaleidoscopeShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load kaleidoscope fragment shader" );
      }
    }

    ~KaleidoscopeShader() override = default;


    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
   {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "isActive", m_data.isActive },
           // { "intensity", m_data.intensity },
          { "slices", m_data.slices }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", true);
        // m_data.intensity = j.value("intensity", 0.2f);
        m_data.slices = j.value("slices", 6.f);
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }


    E_ShaderType getType() const override { return E_ShaderType::E_KaleidoscopeShader; }

    void update( const sf::Time& deltaTime ) override
    {
      // if ( m_data.automateTime )
      //   m_data.time = static_cast< float >( ::clock() ) / CLOCKS_PER_SEC;
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Cosmic-Kaleidoscope" ) )
      {
        ImGui::Checkbox( "Polar-Kaleido Active##1", &m_data.isActive );

        ImGui::SeparatorText("🌀 Polar Kaleido Shader");
        ImGui::SliderFloat("Master Gain", &m_data.masterGain, 0.f, 1.f);
        ImGui::SliderFloat("Slices", &m_data.slices, 1.f, 32.f);
        ImGui::SliderFloat("Swirl Strength", &m_data.swirlStrength, 0.f, 2.f);
        ImGui::SliderFloat("Swirl Density", &m_data.swirlDensity, 1.f, 50.f);
        ImGui::SliderFloat("Pulse Freq", &m_data.pulseFrequency, 1.f, 50.f);
        ImGui::SliderFloat("Pulse Speed", &m_data.pulseSpeed, 0.1f, 10.f);
        ImGui::SliderFloat("Pulse Strength", &m_data.pulseStrength, 0.f, 1.f);
        ImGui::SliderFloat("Angle Steps", &m_data.angleSteps, 1.f, 64.f);
        ImGui::SliderFloat("Radial Stretch", &m_data.radialStretch, 0.2f, 2.0f);
        ImGui::SliderFloat("Noise Strength", &m_data.noiseStrength, 0.f, 1.0f);

        ImGui::SeparatorText( "Easings" );
        m_easings.drawMenu();
        ImGui::SeparatorText( "Midi Triggers" );
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      {
        m_easings.trigger();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize kaleidoscope texture" );
        }
      }

      m_shader.setUniform("u_texture", inputTexture.getTexture());
      m_shader.setUniform("u_time", m_globalInfo.elapsedTimeSeconds);
      m_shader.setUniform("u_intensity", m_data.masterGain);
      m_shader.setUniform("u_resolution", sf::Vector2f(inputTexture.getSize()));

      // Custom control knobs
      m_shader.setUniform("u_kaleidoSlices", m_data.slices);
      m_shader.setUniform("u_swirlStrength", m_data.swirlStrength);
      m_shader.setUniform("u_swirlDensity", m_data.swirlDensity);
      m_shader.setUniform("u_angularPulseFreq", m_data.pulseFrequency);
      m_shader.setUniform("u_pulseStrength", m_data.pulseStrength);
      m_shader.setUniform("u_pulseSpeed", m_data.pulseSpeed);
      m_shader.setUniform("u_angleSteps", m_data.angleSteps);
      m_shader.setUniform("u_radialStretch", m_data.radialStretch);
      m_shader.setUniform("u_noiseStrength", m_data.noiseStrength);

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    KaleidoscopeData_t m_data;
    TimeEasing m_easings;
    MidiNoteControl m_midiNoteControl;

    TimedCursorPosition m_timedCursor;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform float u_time;
uniform float u_intensity;
uniform float u_kaleidoSlices;
uniform vec2 u_resolution;

// Optional extras
uniform float u_swirlStrength;
uniform float u_swirlDensity;
uniform float u_angularPulseFreq;
uniform float u_pulseStrength;
uniform float u_pulseSpeed;
uniform float u_angleSteps;
uniform float u_radialStretch;
uniform float u_noiseStrength;

// GLSL noise function – simple hash-based fake noise
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

const float TAU = 6.28318530718;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    vec2 center = vec2(0.5);
    vec2 offset = uv - center;

    float radius = length(offset);
    float angle = atan(offset.y, offset.x);

    // ------------------------
    // 1. Radial Stretch
    radius = pow(radius, u_radialStretch);

    // 2. Kaleidoscope Mirror Slice
    float normAngle = angle / TAU;
    normAngle = fract(normAngle * u_kaleidoSlices) * TAU;

    // 3. Swirl Overlay
    normAngle += sin(radius * u_swirlDensity + u_time) * u_swirlStrength;

    // 4. Angular Pulse
    radius += sin(normAngle * u_angularPulseFreq - u_time * u_pulseSpeed) * u_pulseStrength;

    // 5. Angle Quantization
    normAngle = floor(normAngle * u_angleSteps) / u_angleSteps;

    // 6. Optional Noise Distortion
    vec2 noiseUV = uv * 5.0 + u_time * 0.1;
    float n = noise(noiseUV);
    normAngle += n * u_noiseStrength;

    // Convert back to cartesian space
    vec2 warpedOffset = vec2(cos(normAngle), sin(normAngle)) * radius;
    //vec2 warpedUV = center + warpedOffset;
    vec2 warpedUV = mix(uv, center + warpedOffset, u_intensity);

    // Final pixel
    gl_FragColor = texture2D(u_texture, warpedUV);
})";

  };

}