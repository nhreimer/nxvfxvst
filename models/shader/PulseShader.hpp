#pragma once

namespace nx
{

  struct PulseData_t
  {
    bool isActive { false };
    float threshold { 0.8f };

    float glowIntensity { 1.2f };

    float pulseDecay { -5.f };
    float burstMultiplier { 1.5f };
  };

  class PulseShader final : public IShader
  {
  public:

    explicit PulseShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      assert( m_brightPassShader.loadFromMemory( m_brightPassFragmentShader, sf::Shader::Type::Fragment ) );
      assert( m_compositeShader.loadFromMemory( m_bloomCompositeFragmentShader, sf::Shader::Type::Fragment ) );
      LOG_INFO( "loaded bloom shaders" );
    }

    ~PulseShader() override = default;
    void update( const sf::Time &deltaTime ) override {}
    void trigger(const Midi_t &midi) override
    {
      m_lastTriggerTime = m_clock.getElapsedTime().asSeconds();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Pulse" ) )
      {
        ImGui::Checkbox( "Pulse Active##1", &m_data.isActive );

        ImGui::SliderFloat("Pulse Threshold##1", &m_data.threshold, 0.0f, 1.0f);

        ImGui::SliderFloat("Glow Intensity##1", &m_data.glowIntensity, 0.0f, 3.0f);
        ImGui::SliderFloat("Pulse Pulse Decay##1", &m_data.pulseDecay, -20.0f, 0.0f);
        ImGui::SliderFloat("Pulse Burst Mult##1", &m_data.burstMultiplier, 0.0f, 5.0f);

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture) override
    {
      ensureTextureSize();

      const float time = m_clock.getElapsedTime().asSeconds();
      const float pulse = std::exp( m_data.pulseDecay * ( time - m_lastTriggerTime ) );
      m_data.glowIntensity = pulse * m_data.burstMultiplier; // Burst of glow!

      // 1. BRIGHT PASS
      m_brightPassShader.setUniform("texture", inputTexture.getTexture());
      m_brightPassShader.setUniform("threshold", m_data.threshold);

      m_brightTexture.clear();
      m_brightTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_brightPassShader);
      m_brightTexture.display();

      // 2. COMPOSITE PASS
      m_compositeShader.setUniform("baseTexture", inputTexture.getTexture());
      m_compositeShader.setUniform("bloomTexture", m_brightTexture.getTexture());
      m_compositeShader.setUniform("intensity", m_data.glowIntensity);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_compositeShader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    void ensureTextureSize()
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        assert( m_outputTexture.resize( m_globalInfo.windowSize ) );
        assert( m_brightTexture.resize( m_globalInfo.windowSize ) );
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    PulseData_t m_data;

    sf::Clock m_clock;
    float m_lastTriggerTime { INT32_MIN };

    sf::Shader m_brightPassShader;
    sf::Shader m_compositeShader;

    sf::RenderTexture m_brightTexture;
    sf::RenderTexture m_outputTexture;

    const static inline std::string m_brightPassFragmentShader = R"(uniform sampler2D texture;
uniform float threshold; // e.g. 0.8

void main() {
    vec4 color = texture2D(texture, gl_FragCoord.xy / vec2(textureSize(texture, 0)));
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); // Luminance
    gl_FragColor = brightness > threshold ? color : vec4(0.0);
})";

    const static inline std::string m_bloomCompositeFragmentShader = R"(uniform sampler2D baseTexture;   // original image
uniform sampler2D bloomTexture;  // blurred glow

uniform float intensity;         // blend strength

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(baseTexture, 0));
    vec4 base = texture2D(baseTexture, uv);
    vec4 glow = texture2D(bloomTexture, uv);
    gl_FragColor = base + glow * intensity;
})";
  };

}