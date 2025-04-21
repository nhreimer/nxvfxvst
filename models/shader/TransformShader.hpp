#pragma once

namespace nx
{
  class TransformShader final : public IShader
  {
    struct TransformData_t
    {
      bool isActive { true };
      float rotationDegrees { 0.f };
      sf::Vector2f shift { 0.f, 0.f };
      sf::Vector2f swirlCenter { 0.f, 0.f };
      bool flipX { false };
      bool flipY { false };
      float swirlStrength { 0.f };
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

          m_timedCursorShift.setPosition( calibrated );
        }

        ImGui::SliderFloat("Rotation (deg)", &m_data.rotationDegrees, -180.f, 180.f);
        ImGui::SliderFloat("Scale", &m_data.scale, 0.1f, 2.0f);

        if ( ImGui::SliderFloat("Swirl Center X", &m_data.swirlCenter.x, -1.f, 1.f) ||
             ImGui::SliderFloat("Swirl Center Y", &m_data.swirlCenter.y, -1.f, 1.f) )
        {
          const sf::Vector2f calibrated
          {
            ( m_data.swirlCenter.x + 0.5f ) * static_cast< float >(m_globalInfo.windowSize.x),
            ( m_data.swirlCenter.y + 0.5f ) * static_cast< float >(m_globalInfo.windowSize.y)
          };

          m_timedCursorSwirl.setPosition( calibrated );
        }

        ImGui::SliderFloat("Swirl Strength", &m_data.swirlStrength, 0.f, 10.f);

        ImGui::Checkbox("Flip X", &m_data.flipX);
        ImGui::SameLine();
        ImGui::Checkbox("Flip Y", &m_data.flipY);

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursorShift.hasExpired() )
        m_timedCursorShift.drawPosition();

      if ( !m_timedCursorSwirl.hasExpired() )
        m_timedCursorSwirl.drawPosition();
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

      m_shader.setUniform("u_rotation", sf::degrees(m_data.rotationDegrees).asRadians());
      m_shader.setUniform("u_swirlCenter", m_data.swirlCenter);
      m_shader.setUniform("u_swirlStrength", m_data.swirlStrength);
      m_shader.setUniform("u_flipX", m_data.flipX);
      m_shader.setUniform("u_flipY", m_data.flipY);

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

    TimedCursorPosition m_timedCursorShift;
    TimedCursorPosition m_timedCursorSwirl;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform vec2 u_offset;
uniform float u_scale;
uniform float u_rotation; // in radians
uniform vec2 u_swirlCenter;
uniform float u_swirlStrength;
uniform bool u_flipX;
uniform bool u_flipY;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;

    // Screen flip
    if (u_flipX) uv.x = 1.0 - uv.x;
    if (u_flipY) uv.y = 1.0 - uv.y;

    // Apply scale and center the effect
    vec2 center = vec2(0.5, 0.5);
    uv = (uv - center) / u_scale + center;

    // Apply rotation around center
    float cosAngle = cos(u_rotation);
    float sinAngle = sin(u_rotation);
    uv -= center;
    uv = mat2(cosAngle, -sinAngle, sinAngle, cosAngle) * uv;
    uv += center;

    // Apply swirl around custom point
    vec2 swirlOffset = uv - u_swirlCenter;
    float dist = length(swirlOffset);
    float angle = u_swirlStrength * dist;
    float cs = cos(angle);
    float sn = sin(angle);
    swirlOffset = mat2(cs, -sn, sn, cs) * swirlOffset;
    uv = u_swirlCenter + swirlOffset;

    // Final offset
    uv += u_offset;

    vec4 color = texture2D(u_texture, uv);
    gl_FragColor = color;
})";

  };
}