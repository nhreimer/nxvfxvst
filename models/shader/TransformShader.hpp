#pragma once

namespace nx
{
  class TransformShader final : public IShader
  {
    struct TransformData_t
    {
      bool isActive { true };
      sf::Vector2f shift { 0.f, 0.f };
      float scale { 1.0f };
    };

  public:

    explicit TransformShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load transform fragment shader" );
      }
      else
      {
        LOG_DEBUG( "Transform fragment shader loaded successfully" );
      }
    }

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_TransformShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Transform") )
      {
        if ( ImGui::SliderFloat("Offset X", &m_data.shift.x, -1.f, 1.f) ||
             ImGui::SliderFloat("Offset Y", &m_data.shift.y, -1.f, 1.f) )
        {
          const sf::Vector2f calibrated
          {
            ( m_data.shift.x + 0.5f ) * static_cast< float >(m_globalInfo.windowSize.x),
            ( m_data.shift.y + 0.5f ) * static_cast< float >(m_globalInfo.windowSize.y)
          };

          m_timedCursorPosition.setPosition( calibrated );
        }
        ImGui::SliderFloat("Scale", &m_data.scale, 0.1f, 2.0f);

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursorPosition.hasExpired() )
        m_timedCursorPosition.drawPosition();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture &applyShader(const sf::RenderTexture &inputTexture) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize transform texture" );
        }
        else
        {
          LOG_INFO( "resized transform texture" );
        }
      }

      m_shader.setUniform("u_texture", inputTexture.getTexture());
      m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture.getSize() });
      m_shader.setUniform("u_offset", m_data.shift);
      m_shader.setUniform("u_scale", m_data.scale);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    TransformData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    TimedCursorPosition m_timedCursorPosition;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_offset;     // e.g. (0.1, -0.2)
uniform float u_scale;     // 1.0 = normal, <1 = zoom in, >1 = zoom out
uniform vec2 u_resolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / u_resolution;

    // Center-based scale
    vec2 center = vec2(0.5, 0.5);
    uv = (uv - center) / u_scale + center;

    // Apply screen-space offset
    uv += u_offset;

    gl_FragColor = texture2D(u_texture, uv);
})";

  };
}