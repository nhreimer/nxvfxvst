#pragma once

namespace nx
{

  struct StrobeData_t
  {
    bool isActive { false };
    float flashAmount { 0.1f };
    float flashDecay { -15.f };
  };

  class StrobeShader final : public IShader
  {
  public:
    explicit StrobeShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      assert( m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) );
      LOG_INFO( "loaded strobe shader" );
    }

    ~StrobeShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", SerialHelper::convertShaderTypeToString( getType() ) },
          { "isActive", m_data.isActive },
          { "flashAmount", m_data.flashAmount },
          { "flashDecay", m_data.flashDecay }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.flashAmount = j.value("flashAmount", 0.1f);
      m_data.flashDecay = j.value("flashDecay", -15.f);
    }

    E_ShaderType getType() const override { return E_StrobeShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Strobe" ) )
      {
        ImGui::Checkbox( "Strobe Active##1", &m_data.isActive );

        ImGui::SliderFloat( "Flash Amount##1", &m_data.flashAmount, 0.f, 1.f );
        ImGui::SliderFloat( "Flash Decay##1", &m_data.flashDecay, -20.f, 0.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {
      m_lastTriggerTime = m_clock.getElapsedTime().asSeconds();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.flashAmount > 0.f; };

    [[nodiscard]]
    sf::RenderTexture &applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        assert( m_outputTexture.resize( m_globalInfo.windowSize ) );
        LOG_INFO( "successfully resized strobe texture" );
      }

      const float time = m_clock.getElapsedTime().asSeconds();
      float pulse = exp( m_data.flashDecay * ( time - m_lastTriggerTime ) );
      pulse = std::clamp( pulse, 0.0f, 1.0f );

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform( "flashAmount", pulse );

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    StrobeData_t m_data;

    float m_lastTriggerTime { INT32_MIN };

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float flashAmount; // 0.0 = no flash, 1.0 = full white

void main() {
    vec4 base = texture2D(texture, gl_FragCoord.xy / vec2(textureSize(texture, 0)));
    base.rgb += flashAmount;
    gl_FragColor = base;
})";
  };
}