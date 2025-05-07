#pragma once

#include "models/shader/BlenderShader.hpp"

namespace nx
{

  class ColorShader final : public IShader
  {

#define COLOR_SHADER_PARAMS(X)                                                                       \
X(brightness, float,        1.0f,   0.0f, 5.0f,   "Controls overall brightness", true)               \
X(saturation, float,        1.0f,   0.0f, 5.0f,   "Vibrancy of the colors", true)                    \
X(contrast,   float,        1.0f,   0.0f, 5.0f,   "Increases color separation", true)                \
X(hueShift,   float,        0.0f,  -NX_PI, NX_PI, "Hue shift in radians", true)                      \
X(colorGain,  sf::Glsl::Vec3, sf::Glsl::Vec3(1.f,1.f,1.f), 0.f, 10.0f, "Multipliers per RGB", false) \
X(mixFactor,  float,        1.0f,   0.f,  1.f,    "Mix between original and effects result", true)

    struct ColorData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(COLOR_SHADER_PARAMS)
    };

    enum class E_ColorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(COLOR_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_ColorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(COLOR_SHADER_PARAMS)
    };

  public:

    explicit ColorShader( PipelineContext& context );

    ~ColorShader() override;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_ColorShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override;

    void drawMenu() override;

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * inputTexture) override;

  private:
    PipelineContext& m_ctx;

    ColorData_t m_data;

    sf::Shader m_shader;
    LazyTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    inline static const std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform float u_brightness;      // 1.0 = normal
uniform float u_saturation;      // 1.0 = normal
uniform vec3 u_gain;             // RGB gain
uniform float u_contrast;        // 1.0 = normal
uniform float u_hueShift;        // radians

vec3 shiftHue(vec3 color, float angle) {
    const mat3 toYIQ = mat3( 0.299,  0.587,  0.114,
                             0.596, -0.275, -0.321,
                             0.212, -0.523,  0.311 );
    const mat3 toRGB = mat3( 1.0,  0.956,  0.621,
                            1.0, -0.272, -0.647,
                            1.0, -1.106,  1.703 );

    vec3 yiq = toYIQ * color;
    float hue = atan(yiq.z, yiq.y) + angle;
    float chroma = length(yiq.yz);
    yiq.y = chroma * cos(hue);
    yiq.z = chroma * sin(hue);
    return toRGB * yiq;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_texture, 0));
    vec4 color = texture2D(u_texture, uv);

    // Brightness
    color.rgb *= u_brightness;

    // Contrast
    color.rgb = (color.rgb - 0.5) * u_contrast + 0.5;

    // Gain
    color.rgb *= u_gain;

    // Saturation
    float gray = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    color.rgb = mix(vec3(gray), color.rgb, u_saturation);

    // Hue shift
    color.rgb = shiftHue(color.rgb, u_hueShift);

    gl_FragColor = color;
})";
  };
}