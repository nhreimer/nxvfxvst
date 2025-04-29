#pragma once

#include "helpers/CommonHeaders.hpp"

#include "helpers/ColorHelper.hpp"

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
    explicit DensityHeatMapShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load density heat map fragment shader" );
      }
      else
      {
        LOG_INFO( "Loaded density heat map fragment shader" );
      }

      EXPAND_SHADER_VST_BINDINGS(DENSITY_HEATMAP_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~DensityHeatMapShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum(getType());
      EXPAND_SHADER_PARAMS_TO_JSON(DENSITY_HEATMAP_SHADER_PARAMS)
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(DENSITY_HEATMAP_SHADER_PARAMS)
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_DensityHeatMapShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Density Heat Map" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        EXPAND_SHADER_IMGUI(DENSITY_HEATMAP_SHADER_PARAMS, m_data)

        ImGui::SeparatorText( "Easings" );
        m_easing.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {
      m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture &applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize density heat map texture" );
        }
      }

      m_shader.setUniform("u_densityTexture", inputTexture.getTexture());
      m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture.getSize() });
      m_shader.setUniform("u_falloff", m_data.falloff.first * m_easing.getEasing() );

      m_outputTexture.clear( sf::Color::Transparent );

      m_shader.setUniform( "u_colorCoolStart", ColorHelper::convertFromVec4( m_data.colorCoolStart.first ) );
      m_shader.setUniform( "u_colorCoolEnd", ColorHelper::convertFromVec4( m_data.colorCoolEnd.first ) );

      m_shader.setUniform( "u_colorWarmStart", ColorHelper::convertFromVec4( m_data.colorWarmStart.first ) );
      m_shader.setUniform( "u_colorWarmEnd", ColorHelper::convertFromVec4( m_data.colorWarmEnd.first ) );

      m_shader.setUniform( "u_colorHotStart", ColorHelper::convertFromVec4( m_data.colorHotStart.first ) );
      m_shader.setUniform( "u_colorHotEnd", ColorHelper::convertFromVec4( m_data.colorHotEnd.first ) );

      m_shader.setUniform( "u_colorMaxStart", ColorHelper::convertFromVec4( m_data.colorMaxStart.first ) );
      m_shader.setUniform( "u_colorMaxEnd", ColorHelper::convertFromVec4( m_data.colorMaxEnd.first ) );

      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    PipelineContext& m_ctx;
    DensityHeatMapData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

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