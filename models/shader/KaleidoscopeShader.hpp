#pragma once

#include <SFML/Graphics/Shader.hpp>

namespace nx
{

  struct KaleidoscopeData_t
  {
    bool isActive { false };

    float centerX { 0.5f };
    float centerY { 0.5f };
    int32_t segments { 0 };
    float time { 0.f };
    float rotationSpeed { 0.1f };

    bool automateTime { false };
  };

  class KaleidoscopeShader final : public IShader
  {

  public:

    explicit KaleidoscopeShader( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {
      assert( m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) );
      LOG_INFO( "loaded kaleidoscope shader" );
    }

    void update( const sf::Time& deltaTime ) override
    {
      if ( m_data.automateTime )
        m_data.time = static_cast< float >( ::clock() ) / CLOCKS_PER_SEC;
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Kaleidoscope" ) )
      {
        ImGui::Checkbox( "Kaleidoscope Active##1", &m_data.isActive );

        ImGui::SliderInt( "Segments##1", &m_data.segments, 0, 255 );
        ImGui::SliderFloat( "Center x##1", &m_data.centerX, -1.0f, 1.0f );
        ImGui::SliderFloat( "Center y##1", &m_data.centerY, -1.0f, 1.0f );
        ImGui::SliderFloat( "Rotate##1", &m_data.time, 0.f, 1.f );

        ImGui::Separator();
        ImGui::Checkbox( "Automate Rotation##1", &m_data.automateTime );
        ImGui::SliderFloat( "Rotation Speed##1", &m_data.rotationSpeed, 0.f, 1.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.segments > 0; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_winfo.windowSize )
      {
        assert( m_outputTexture.resize( m_winfo.windowSize ) );
        LOG_INFO( "successfully resized kaleidoscope texture" );
      }

      m_shader.setUniform( "texture", sf::Shader::CurrentTexture );
      m_shader.setUniform( "resolution", sf::Vector2f( m_winfo.windowSize ) );
      m_shader.setUniform( "center", sf::Vector2f( m_data.centerX, m_data.centerY ) ); // Center in UV space
      m_shader.setUniform( "numSegments", m_data.segments );
      m_shader.setUniform( "time", m_data.time );
      m_shader.setUniform( "rotationSpeed", m_data.rotationSpeed );

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_winfo;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    KaleidoscopeData_t m_data;

    static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;     // Window size
uniform vec2 center;         // Center of the kaleidoscope
uniform int numSegments;     // Number of mirrored segments
uniform float time;          // Optional time to animate
uniform float rotationSpeed; //

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;

    // Offset from center
    vec2 delta = uv - center;

    // Convert to polar coordinates
    float r = length(delta);
    float theta = atan(delta.y, delta.x);

    // Apply time-based rotation (optional)
    theta += time * rotationSpeed; // 0.1 is default

    // Mirror the angle across segment boundaries
    float segmentAngle = 3.14159265 * 2.0 / float(numSegments);
    theta = mod(theta, segmentAngle);
    theta = abs(theta - segmentAngle * 0.5);

    // Convert back to cartesian
    vec2 newDelta = vec2(cos(theta), sin(theta)) * r;
    vec2 sampleUV = center + newDelta;

    // Sample the texture
    vec4 color = texture2D(texture, sampleUV);
    gl_FragColor = color;
})";


  };

}