#pragma once

namespace nx
{

  struct GlitchData_t
  {
    bool isActive { false };

    float glitchStrength { 1.f };       // How intense glitches are (0.0 to 1.0+)
    float glitchAmount { 0.4f };        // How frequent glitches are (0.0 to 1.0)
    float scanlineIntensity { 0.02f };  // Scanline brightness
    float chromaFlickerAmount { 0.4f };  // 0.0 = off, 1.0 = frequent flickers
    float strobeAmount { 0.2f };         // 0.0 = no strobe, 1.0 = frequent
    float pixelJumpAmount { 0.5f };      // 0.0 = off, 1.0 = chaos
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

        ImGui::SliderFloat( "Glitch Strength##1", &m_data.glitchStrength, 0.f, 1.0f );
        ImGui::SliderFloat( "Glitch Amount##1", &m_data.glitchAmount, 0.f, 1.0f );
        ImGui::SliderFloat( "Scanline Strength##1", &m_data.scanlineIntensity, 0.f, 1.0f );

        ImGui::SliderFloat( "Chroma Flicker##1", &m_data.chromaFlickerAmount, 0.f, 1.0f );
        ImGui::SliderFloat( "Strobe##1", &m_data.strobeAmount, 0.f, 1.0f );
        ImGui::SliderFloat( "Pixel Jumps##1", &m_data.pixelJumpAmount, 0.f, 1.0f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    bool isShaderActive() const override { return m_data.isActive && m_data.glitchStrength > 0.f; }

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
      m_shader.setUniform("glitchStrength", m_data.glitchStrength);
      m_shader.setUniform("glitchAmount", m_data.glitchAmount);
      m_shader.setUniform("scanlineIntensity", m_data.scanlineIntensity);

      m_shader.setUniform("chromaFlickerAmount", m_data.chromaFlickerAmount);   // 0.0 = off, 1.0 = frequent flickers
      m_shader.setUniform("strobeAmount", m_data.strobeAmount);          // 0.0 = no strobe, 1.0 = frequent
      m_shader.setUniform("pixelJumpAmount", m_data.pixelJumpAmount);       // 0.0 = off, 1.0 = chaos

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

uniform float glitchStrength;
uniform float glitchAmount;
uniform float scanlineIntensity;

uniform float chromaFlickerAmount;
uniform float strobeAmount;
uniform float pixelJumpAmount;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    float bandCount = 20.0;
    float band = floor(uv.y * bandCount);

    float pulse = 0.5 + 0.5 * sin(time * 2.0);
    float strength = glitchStrength * pulse;
    float amount = glitchAmount;

    float blockThreshold = 1.0 - amount * 0.9;
    float jumpThreshold = 1.0 - amount * 0.8;

    // Block Glitch
    float blockRand = rand(vec2(band, floor(time * 10.0)));
    float blockOffsetX = step(blockThreshold, blockRand) * rand(vec2(band, time)) * 0.2 * strength;
    float blockOffsetY = step(blockThreshold - 0.1, blockRand) * rand(vec2(band + 0.5, time)) * 0.05 * strength;

    uv.x += blockOffsetX;
    uv.y += blockOffsetY;

    // UV Tearing
    float tearIntensity = strength * 0.03;
    uv.x += (rand(vec2(time, uv.y * 100.0)) - 0.5) * tearIntensity;
    uv.y += (rand(vec2(uv.x * 100.0, time)) - 0.5) * tearIntensity;

    // Vertical Jump
    float jump = step(jumpThreshold, rand(vec2(floor(time * 2.0), 5.0))) * rand(vec2(time, 0.0)) * 0.1 * strength;
    uv.y += jump;

    // ✴️ Pixelation Jump (intermittent pixel blocks)
    float pixelBlock = 1.0;
    if (step(1.0 - pixelJumpAmount, rand(vec2(floor(time * 4.0), 1.0))) > 0.5) {
        pixelBlock = floor(rand(vec2(time, 2.0)) * 40.0 + 4.0);
        uv = floor(uv * pixelBlock) / pixelBlock;
    }

    // 🔺 RGB Shift
    vec2 rgbShift = vec2(rand(vec2(time, uv.y)) * 0.005 * strength, 0.0);

    // 🔵 Chromatic Flicker — only sometimes exaggerate the RGB offset
    if (step(1.0 - chromaFlickerAmount, rand(vec2(time * 3.0, 3.0))) > 0.5) {
        rgbShift *= 5.0;
    }

    vec4 r = texture2D(texture, uv + rgbShift);
    vec4 g = texture2D(texture, uv);
    vec4 b = texture2D(texture, uv - rgbShift);

    vec4 color = vec4(r.r, g.g, b.b, 1.0);

    // 🌀 Scanlines
    float scanline = sin(uv.y * resolution.y * 10.0 + time * 40.0) * scanlineIntensity * strength;
    color.rgb += scanline;

    // ⚡ Strobe Flash — full screen bright flash
    if (step(1.0 - strobeAmount, rand(vec2(floor(time * 4.0), 7.0))) > 0.5) {
        color.rgb += vec3(1.0); // white flash
    }

    gl_FragColor = color;
})";

  };

}