#pragma once

#include "models/shader/BlenderShader.hpp"

namespace nx
{
  class TransformShader final : public IShader
  {
#define TRANSFORM_SHADER_PARAMS(X)                                                                 \
X(rotationDegrees, float, 0.f,   -360.f, 360.f, "Rotation applied to the screen", true)            \
X(shift,           sf::Vector2f, sf::Vector2f(0.f, 0.f), 0.f, 0.f, "Screen offset (X, Y)", false)  \
X(flipX,           bool, false,  0, 0, "Horizontal flip toggle", false)                            \
X(flipY,           bool, false,  0, 0, "Vertical flip toggle", false)                              \
X(scale,           float, 1.f,   0.1f, 5.f, "Uniform scale factor for zoom or shrink", true)       \
X(mixFactor,       float, 1.0f,  0.f,  1.f, "Mix between original and effects result", true)

    struct TransformData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(TRANSFORM_SHADER_PARAMS)
    };

    enum class E_TransformParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(TRANSFORM_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_TransformParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(TRANSFORM_SHADER_PARAMS)
    };

  public:

    explicit TransformShader( PipelineContext& context );

    ~TransformShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_TransformShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override;

    void trigger( const AudioDataBuffer& buffer ) override
    {
      m_easing.trigger();
    }

    void drawMenu() override;

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:
    PipelineContext& m_ctx;

    TransformData_t m_data;

    sf::Shader m_shader;
    LazyTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    TimedCursorPosition m_timedCursorShift;
    // TimedCursorPosition m_timedCursorStretch;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform vec2 u_offset;
uniform float u_scale;
uniform float u_rotation; // radians
uniform bool u_flipX;
uniform bool u_flipY;

void main() {
    // Pixel-space coordinates
    vec2 pixelCoord = gl_FragCoord.xy;

    // Flip in screen space (if needed)
    if (u_flipX) pixelCoord.x = u_resolution.x - pixelCoord.x;
    if (u_flipY) pixelCoord.y = u_resolution.y - pixelCoord.y;

    // Center of screen in pixels
    vec2 center = 0.5 * u_resolution;

    // Translate to origin, scale, rotate, then translate back
    vec2 pos = pixelCoord - center;
    pos /= u_scale;

    float cosA = cos(u_rotation);
    float sinA = sin(u_rotation);
    pos = mat2(cosA, -sinA, sinA, cosA) * pos;

    pos += center;

    // Apply offset in pixels (not UV!)
    pos += vec2(-u_offset.x * u_resolution.x, u_offset.y * u_resolution.y);

    // Convert back to normalized UVs
    vec2 uv = pos / u_resolution;

    gl_FragColor = texture2D(u_texture, uv);
})";

  };
}