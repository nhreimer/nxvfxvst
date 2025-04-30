#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/shader/BlenderShader.hpp"

namespace nx
{
  class RippleShader final : public IShader
  {

#define RIPPLE_SHADER_PARAMS(X)                                                                           \
X(rippleCenterX, float, 0.5f,  0.f, 1.f,   "Horizontal origin of ripple (0.0 = left, 1.0 = right)", true) \
X(rippleCenterY, float, 0.5f,  0.f, 1.f,   "Vertical origin of ripple (0.0 = top, 1.0 = bottom)", true)   \
X(amplitude,     float, 0.05f, 0.f, 1.f,   "[CALC] Ripple strength, typically eased", true)               \
X(frequency,     float, 10.f,  1.f, 100.f, "Wave density across the screen", true)                        \
X(speed,         float, 0.f,   0.f, 50.f,  "Wave movement speed over time", true)                         \
X(mixFactor,     float, 1.0f,  0.f, 1.f,   "Mix between original and effects result", true)
    struct RippleData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(RIPPLE_SHADER_PARAMS)
    };

    enum class E_RippleParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(RIPPLE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_RippleParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(RIPPLE_SHADER_PARAMS)
    };

  public:

    explicit RippleShader( PipelineContext& context );

    ~RippleShader() override;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json& j) override;

    E_ShaderType getType() const override { return E_ShaderType::E_RippleShader; }

    void drawMenu() override;

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t& midi ) override;

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture & applyShader( const sf::RenderTexture &inputTexture ) override;

  private:
    PipelineContext& m_ctx;
    RippleData_t m_data;

    sf::Clock m_clock;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;
    TimedCursorPosition m_timedCursor;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;

uniform vec2 rippleCenter;
uniform float amplitude;   // base amplitude
uniform float frequency;
uniform float speed;

uniform float intensity;   // added: easing-controlled value [0..1]

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec2 delta = uv - rippleCenter;
    float dist = length(delta);

    // Ripple effect, scaled by intensity
    float ripple = sin(dist * frequency - time * speed) * amplitude;// * intensity;

    vec2 dir = normalize(delta);
    uv += dir * ripple;

    gl_FragColor = texture2D(texture, uv);
})";
  };

}