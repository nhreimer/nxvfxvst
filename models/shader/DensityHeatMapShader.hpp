#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/shader/BlenderShader.hpp"

namespace nx
{
  class DensityHeatMapShader final : public IShader
  {

#define DENSITY_HEATMAP_SHADER_PARAMS(X)                                                                 \
X(falloff,        float,     0.2f,   0.0f, 1.0f,   "Falloff intensity over distance", true)              \
X(colorCoolStart, sf::Color, sf::Color(0, 0, 0),        0, 255, "Gradient: Cool start color", false)     \
X(colorCoolEnd,   sf::Color, sf::Color(0, 0, 255),      0, 255, "Gradient: Cool end color", false)       \
X(colorWarmStart, sf::Color, sf::Color(0, 0, 255),      0, 255, "Gradient: Warm start color", false)     \
X(colorWarmEnd,   sf::Color, sf::Color(0, 255, 255),    0, 255, "Gradient: Warm end color", false)       \
X(colorHotStart,  sf::Color, sf::Color(0, 255, 255),    0, 255, "Gradient: Hot start color", false)      \
X(colorHotEnd,    sf::Color, sf::Color(255, 255, 0),    0, 255, "Gradient: Hot end color", false)        \
X(colorMaxStart,  sf::Color, sf::Color(255, 255, 0),    0, 255, "Gradient: Max start color", false)      \
X(colorMaxEnd,    sf::Color, sf::Color(255, 0, 0),      0, 255, "Gradient: Max end color", false)        \
X(mixFactor,  float,        1.0f,   0.f,  1.f,    "Mix between original and effects result", true)

    struct DensityHeatMapData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(DENSITY_HEATMAP_SHADER_PARAMS)
    };

    enum class E_DensityHeatMapParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(DENSITY_HEATMAP_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_DensityHeatMapParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(DENSITY_HEATMAP_SHADER_PARAMS)
    };

  public:
    explicit DensityHeatMapShader( PipelineContext& context );

    ~DensityHeatMapShader() override = default;

    void destroyTextures() override
    {
      m_outputTexture.destroy();
      m_blender.destroyTextures();
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json& j) override;

    E_ShaderType getType() const override { return E_ShaderType::E_DensityHeatMapShader; }

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
    sf::RenderTexture * applyShader( const sf::RenderTexture * inputTexture ) override;

  private:
    PipelineContext& m_ctx;
    DensityHeatMapData_t m_data;

    sf::Shader m_shader;
    LazyTexture m_outputTexture;
    BlenderShader m_blender;

    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_densityTexture;
uniform vec2 u_resolution;
uniform float u_falloff;

uniform vec3 u_colorCoolStart;
uniform vec3 u_colorCoolEnd;

uniform vec3 u_colorWarmStart;
uniform vec3 u_colorWarmEnd;

uniform vec3 u_colorHotStart;
uniform vec3 u_colorHotEnd;

uniform vec3 u_colorMaxStart;
uniform vec3 u_colorMaxEnd;

vec3 heatmapColor(float t) {
    t = clamp(t, 0.0, 1.0);
    if (t < 0.25) return mix(u_colorCoolStart, u_colorCoolEnd, t / 0.25);
    if (t < 0.5)  return mix(u_colorWarmStart, u_colorWarmEnd, (t - 0.25) / 0.25);
    if (t < 0.75) return mix(u_colorHotStart, u_colorHotEnd, (t - 0.5) / 0.25);
    return mix(u_colorMaxStart, u_colorMaxEnd, (t - 0.75) / 0.25);
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    float brightness = texture2D(u_densityTexture, uv).a; // use alpha channel
    brightness = pow(brightness, u_falloff); // higher = tighter, lower = wider

    vec3 color = heatmapColor(brightness);
    gl_FragColor = vec4(color, brightness); // output with same brightness
})";
  };
}