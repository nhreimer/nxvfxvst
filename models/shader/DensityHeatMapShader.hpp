#pragma once

#include "helpers/CommonHeaders.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  struct DensityHeatMapData_t
  {
    bool isActive { true };
    float falloff { 0.2f };
  };

  class DensityHeatMapShader final : public IShader
  {
  public:
    explicit DensityHeatMapShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load strobe fragment shader" );
      }
      else
      {
        LOG_INFO( "Loaded density heat map fragment shader" );
      }
    }

    ~DensityHeatMapShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", SerialHelper::serializeEnum( getType() ) },
          { "isActive", m_data.isActive },
          { "falloff", m_data.falloff }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", false);
        m_data.falloff = j.value("falloff", 0.2f);
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
        ImGui::Checkbox( "Heat Map Active##1", &m_data.isActive );
        ImGui::SliderFloat("Density Falloff", &m_data.falloff, 0.1f, 5.0f);

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {}

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture &applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize density heat map texture" );
        }
      }

      m_shader.setUniform("u_densityTexture", inputTexture.getTexture());
      m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture.getSize() });
      m_shader.setUniform("u_falloff", m_data.falloff);

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    DensityHeatMapData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_densityTexture;
uniform vec2 u_resolution;
uniform float u_falloff;

vec3 heatmapColor(float t) {
    t = clamp(t, 0.0, 1.0);
    if (t < 0.25) return mix(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), t / 0.25);
    if (t < 0.5)  return mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), (t - 0.25) / 0.25);
    if (t < 0.75) return mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), (t - 0.5) / 0.25);
    return mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (t - 0.75) / 0.25);
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