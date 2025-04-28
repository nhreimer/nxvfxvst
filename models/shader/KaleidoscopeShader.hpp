#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class KaleidoscopeShader final : public IShader
  {

// easings
// slices, angleSteps, and swirlStrength

#define KALEIDOSCOPE_SHADER_PARAMS(X)                                                                \
X(masterGain,     float, 0.1f,  0.f,  5.f,   "Overall kaleido intensity multiplier", true)           \
X(slices,         float, 6.f,   3.f,  24.f,  "Number of kaleidoscope slices", true)                  \
X(swirlStrength,  float, 1.f,   0.f,  5.f,   "Amount of swirl distortion per slice", true)           \
X(swirlDensity,   float, 1.f,   0.f,  10.f,  "Swirl frequency (tightness of rotation)", true)        \
X(pulseStrength,  float, 0.2f,  0.f,  5.f,   "Strength of pulsing wave distortions", true)           \
X(pulseFrequency, float, 10.f,  0.f,  50.f,  "How often pulses occur (Hz)", true)                    \
X(pulseSpeed,     float, 5.f,   0.f,  50.f,  "How quickly pulses move through the shape", true)      \
X(angleSteps,     float, 32.f,  3.f,  128.f, "How many radial segments are processed", true)         \
X(radialStretch,  float, 1.f,   0.1f, 3.f,   "Stretch factor on the radial axis", true)              \
X(noiseStrength,  float, 0.5f,  0.f,  2.f,   "Amount of Perlin-like distortion overlay", true)       \
X(mixFactor,      float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct KaleidoscopeData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(KALEIDOSCOPE_SHADER_PARAMS)
    };

    enum class E_KaleidoParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(KALEIDOSCOPE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_KaleidoParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(KALEIDOSCOPE_SHADER_PARAMS)
    };

  public:

    explicit KaleidoscopeShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load kaleidoscope fragment shader" );
      }
      else
      {
        LOG_INFO( "Kaleidoscope fragment shader loaded" );
      }

      EXPAND_SHADER_VST_BINDINGS(KALEIDOSCOPE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~KaleidoscopeShader() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum(getType());
      EXPAND_SHADER_PARAMS_TO_JSON(KALEIDOSCOPE_SHADER_PARAMS)
      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(KALEIDOSCOPE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }


    E_ShaderType getType() const override { return E_ShaderType::E_KaleidoscopeShader; }

    void update( const sf::Time& deltaTime ) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Cosmic-Kaleidoscope Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        KALEIDOSCOPE_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::SeparatorText( "Easings" );
        m_easing.drawMenu();
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
        m_easing.trigger();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize kaleidoscope texture" );
        }
        else
        {
          LOG_INFO( "kaleidoscope texture resized" );
        }
      }

      m_shader.setUniform( "u_time", m_easing.getEasing() );

      m_shader.setUniform("u_texture", inputTexture.getTexture());
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

      return m_blender.applyShader( inputTexture,
                                    m_outputTexture,
                                    m_data.mixFactor );
    }

  private:

    PipelineContext& m_ctx;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    KaleidoscopeData_t m_data;

    BlenderShader m_blender;
    TimeEasing m_easing;
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