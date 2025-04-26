#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class RumbleShader final : public IShader
  {

#define RUMBLE_SHADER_PARAMS(X)                                                                 \
X(rumbleStrength, float, 20.f, 0.f, 100.f,  "Max pixel offset during shake")                    \
X(frequency,       float, 40.f, 1.f, 200.f, "Shake frequency in Hz")                            \
X(pulseDecay,      float, -5.f, -50.f, 0.f, "Decay rate for shake pulses")                      \
X(direction, sf::Vector2f, sf::Vector2f(1.f, 1.f), 0.f, 0.f, "Shake axis direction (x, y)")     \
X(useNoise,        bool, false, 0, 0,      "Enable procedural noise mode instead of sine wave") \
X(modAmplitude,    float, 0.5f, 0.f, 2.f,  "Amount of wobble modulation over time")             \
X(modFrequency,    float, 10.f, 0.f, 50.f, "Speed of wobble modulation")                        \
X(colorDesync,     float, 1.0f, 0.f, 5.f,  "RGB offset multiplier (screen tearing)")            \
X(baseColorDesync, float, 0.25f, 0.f, 2.f, "Base chroma offset before modulation")              \
X(maxColorDesync,  float, 0.25f, 0.f, 2.f, "Max chroma offset from center")                     \
X(mixFactor,       float, 1.0f,    0.f,   1.f, "Mix between original and effects result")

    struct RumbleData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(RUMBLE_SHADER_PARAMS)
    };

    enum class E_RumbleParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(RUMBLE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_RumbleParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(RUMBLE_SHADER_PARAMS)
    };

  public:

    explicit RumbleShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load rumble fragment shader" );
      }
      else
      {
        LOG_DEBUG( "Rumble fragment shader loaded successfully" );
      }
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(RUMBLE_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize( const nlohmann::json &j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(RUMBLE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_RumbleShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Rumble Options" ) )
      {
        ImGui::Checkbox("Active", &m_data.isActive);
        auto& STRUCT_REF = m_data;
        RUMBLE_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture &applyShader(const sf::RenderTexture &inputTexture) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize rumble texture" );
        }
      }

      const float time = m_clock.getElapsedTime().asSeconds();
      const float pulse = m_easing.getEasing();

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));
      m_shader.setUniform("time", time);

      m_shader.setUniform("rumbleStrength", m_data.rumbleStrength);
      m_shader.setUniform("frequency", m_data.frequency);
      m_shader.setUniform("pulseValue", pulse); // now driven by easing
      m_shader.setUniform("direction", sf::Glsl::Vec2(m_data.direction));
      m_shader.setUniform("useNoise", m_data.useNoise);

      m_shader.setUniform("modAmplitude", m_data.modAmplitude);
      m_shader.setUniform("modFrequency", m_data.modFrequency);
      m_shader.setUniform("colorDesync", pulse * m_data.maxColorDesync + m_data.baseColorDesync);
      //m_shader.setUniform("colorDesync", m_data.colorDesync);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                                    m_outputTexture,
                                    m_data.mixFactor );
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    RumbleData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;

uniform float rumbleStrength;
uniform float frequency;
uniform float pulseValue;
uniform vec2 direction;
uniform bool useNoise;

// Amplitude modulation
uniform float modAmplitude;
uniform float modFrequency;

// RGB offset
uniform float colorDesync;

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    float ampMod = 1.0 + sin(time * modFrequency) * modAmplitude;

    float shakeX = 0.0;
    float shakeY = 0.0;

    if (useNoise) {
        shakeX = (hash(time * frequency) - 0.5) * 2.0 * direction.x;
        shakeY = (hash(time * frequency + 100.0) - 0.5) * 2.0 * direction.y;
    } else {
        shakeX = sin(time * frequency) * direction.x;
        shakeY = cos(time * frequency) * direction.y;
    }

    vec2 shake = vec2(shakeX, shakeY) * (rumbleStrength * ampMod / resolution) * pulseValue;

    // Color desync (red and blue shift opposite directions)
    vec2 redUV = uv + shake * colorDesync;
    vec2 greenUV = uv;
    vec2 blueUV = uv - shake * colorDesync;

    float r = texture2D(texture, redUV).r;
    float g = texture2D(texture, greenUV).g;
    float b = texture2D(texture, blueUV).b;

    gl_FragColor = vec4(r, g, b, 1.0);
})";

  };
}