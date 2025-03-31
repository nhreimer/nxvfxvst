#pragma once

namespace nx
{

  struct GlitchData_t
  {
    bool isActive { false };
    float intensity { 0.5f };
  };

  class GlitchShader final : public IShader
  {
  public:
    explicit GlitchShader( const WindowInfo_t& winfo )
      : m_winfo( winfo )
    {
      assert( m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) );
      LOG_INFO( "loaded glitch shader" );
    }

    ~GlitchShader() override = default;

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Glitch" ) )
      {
        ImGui::Checkbox( "Glitch Active##1", &m_data.isActive );

        ImGui::SliderFloat( "Intensity##1", &m_data.intensity, 0.f, 1.0f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    bool isShaderActive() const override { return m_data.isActive && m_data.intensity > 0.f; }

    sf::RenderTexture & applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_winfo.windowSize )
      {
        assert( m_outputTexture.resize( m_winfo.windowSize ) );
        LOG_INFO( "successfully resized glitch texture" );
      }

      const float time = m_clock.getElapsedTime().asSeconds();

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));
      m_shader.setUniform("time", time);
      m_shader.setUniform("intensity", m_data.intensity); // Change this to increase glitch strength

      m_outputTexture.clear();
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const WindowInfo_t& m_winfo;

    GlitchData_t m_data;

    sf::Shader m_shader;
    sf::Clock m_clock;
    sf::RenderTexture m_outputTexture;

    static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;
uniform float intensity;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    float scanline = sin(uv.y * resolution.y * 10.0 + time * 40.0) * 0.01;

    // Horizontal strip offset
    float yGlitch = step(0.95, rand(vec2(floor(uv.y * 20.0), time))) * rand(vec2(uv.y, time)) * 0.1 * intensity;
    uv.x += yGlitch;

    // RGB channel offsets
    vec2 glitchOffset = vec2(rand(vec2(time, uv.y)) * 0.005 * intensity, 0.0);
    vec4 r = texture2D(texture, uv + glitchOffset);
    vec4 g = texture2D(texture, uv);
    vec4 b = texture2D(texture, uv - glitchOffset);

    vec4 color = vec4(r.r, g.g, b.b, 1.0);

    // Apply scanlines
    color.rgb += scanline;

    gl_FragColor = color;
})";

  };

}