#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  struct KaleidoscopeData_t
  {
    bool isActive { true };
    float intensity { 0.2f };

    // Slices | Vibe
    //--------|--------------------------
    // 3–4    | Triangular shard symmetry
    // 6–8    | Mandala snowflake bloom
    // 12+    | Fractal kaleido insanity

    float slices { 6.f };
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
           { "intensity", m_data.intensity },
          { "slices", m_data.slices }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", true);
        m_data.intensity = j.value("intensity", 0.2f);
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
      if ( ImGui::TreeNode( "Polar-Kaleidoscope" ) )
      {
        ImGui::Checkbox( "Polar-Kaleido Active##1", &m_data.isActive );
        ImGui::SliderFloat( "Polar-Kaleido Intensity", &m_data.intensity, 0.f, 1.f );
        ImGui::SliderFloat( "Kaleido Slices", &m_data.slices, 1.f, 24.f, "%.1f" );

        // ImGui::Separator();
        // ImGui::Checkbox( "Automate Rotation##1", &m_data.automateTime );
        // ImGui::SliderFloat( "Rotation Speed##1", &m_data.rotationSpeed, 0.f, 1.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

    void trigger( const Midi_t& midi ) override
    {

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
      m_shader.setUniform("u_intensity", m_data.intensity);
      m_shader.setUniform("u_kaleidoSlices", m_data.slices);
      m_shader.setUniform("u_resolution", sf::Vector2f(inputTexture.getSize()));


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

    TimedCursorPosition m_timedCursor;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform float u_time;
uniform float u_intensity;
uniform float u_kaleidoSlices; // number of mirrored slices
uniform vec2 u_resolution;

const float TAU = 6.28318530718;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    vec2 center = vec2(0.5, 0.5);
    vec2 offset = uv - center;

    float radius = length(offset);
    float angle = atan(offset.y, offset.x);

    // Kaleido magic
    float normalizedAngle = angle / TAU;         // range [-0.5, 0.5]
    normalizedAngle = fract(normalizedAngle * u_kaleidoSlices) * TAU;

    // Optional angular warp (swirliness)
    float warp = sin(radius * 30.0 - u_time * 3.0) * u_intensity;
    float finalAngle = normalizedAngle + warp;

    vec2 warpedOffset = vec2(cos(finalAngle), sin(finalAngle)) * radius;
    vec2 warpedUV = center + warpedOffset;

    gl_FragColor = texture2D(u_texture, warpedUV);
})";

  };

}