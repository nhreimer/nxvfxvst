#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/shader/BlenderShader.hpp"

namespace nx
{

  class RumbleShader final : public IShader
  {

#define RUMBLE_SHADER_PARAMS(X)                                                                       \
X(rumbleStrength, float, 20.f, 0.f, 100.f,  "Max pixel offset during shake", true)                    \
X(frequency,       float, 40.f, 1.f, 200.f, "Shake frequency in Hz", true)                            \
X(pulseDecay,      float, -5.f, -50.f, 0.f, "Decay rate for shake pulses", true)                      \
X(direction, sf::Vector2f, sf::Vector2f(1.f, 1.f), 0.f, 0.f, "Shake axis direction (x, y)", false)    \
X(useNoise,        bool, false, 0, 0,      "Enable procedural noise mode instead of sine wave", true) \
X(modAmplitude,    float, 0.5f, 0.f, 2.f,  "Amount of wobble modulation over time", true)             \
X(modFrequency,    float, 10.f, 0.f, 50.f, "Speed of wobble modulation", true)                        \
X(colorDesync,     float, 1.0f, 0.f, 5.f,  "RGB offset multiplier (screen tearing)", true)            \
X(baseColorDesync, float, 0.25f, 0.f, 2.f, "Base chroma offset before modulation", true)              \
X(maxColorDesync,  float, 0.25f, 0.f, 2.f, "Max chroma offset from center", true)                     \
X(mixFactor,       float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

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

    explicit RumbleShader( PipelineContext& context );

    ~RumbleShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json &j ) override;

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_RumbleShader; }

    void drawMenu() override;

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:

    PipelineContext& m_ctx;
    RumbleData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    LazyTexture m_outputTexture;

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