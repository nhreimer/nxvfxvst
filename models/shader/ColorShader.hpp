#pragma once

namespace nx
{

  class ColorShader final : public IShader
  {

    struct ColorData_t
    {
      bool isActive { true };

      float brightness { 1.0f };                      // 1.0 = normal
      float saturation { 1.0f };                      // 1.0 = normal
      sf::Glsl::Vec3 colorGain { 1.f, 1.f, 1.f }; // RGB gain
      float contrast { 1.0f };                        // 1.0 = normal
      float hueShift { 0.0f };                        // in radians
    };

  public:

    explicit ColorShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load color fragment shader" );
      }
      else
      {
        LOG_DEBUG( "Color fragment shader loaded successfully" );
      }
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "isActive", m_data.isActive },
        { "brightness", m_data.brightness },
        { "saturation", m_data.saturation },
        { "hueShift", m_data.hueShift },
        { "easing", m_easing.serialize() },
        { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.brightness = j.value("brightness", 1.0f);
      m_data.saturation = j.value("saturation", 1.0f);
      m_data.contrast = j.value("contrast", 1.0f);
      m_data.hueShift = j.value("hueShift", 0.0f);
      // TODO: add color gain
      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_ColorShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Color" ) )
      {
        ImGui::SliderFloat( "Brightness", &m_data.brightness, 0.f, 2.f );
        ImGui::SliderFloat( "Saturation", &m_data.saturation, 0.f, 2.f );
        ImGui::SliderFloat( "Contrast", &m_data.contrast, 0.f, 2.f );
        ImGui::SliderFloat( "Hue", &m_data.hueShift, -NX_PI, NX_PI );

        ImGui::SeparatorText( "Color Gain" );
        ImGui::SliderFloat( "Gain Red", &m_data.colorGain.x, 0.f, 10.f );
        ImGui::SliderFloat( "Gain Green", &m_data.colorGain.y, 0.f, 10.f );
        ImGui::SliderFloat( "Gain Blue", &m_data.colorGain.z, 0.f, 10.f );

        ImGui::SeparatorText( "Easings" );
        m_easing.drawMenu();

        ImGui::SeparatorText( "Midi Trigger" );
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
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
          LOG_ERROR( "failed to resize color texture" );
        }
        else
        {
          LOG_INFO( "color transform texture" );
        }
      }

      const auto easing = m_easing.getEasing();

      m_shader.setUniform("u_texture", inputTexture.getTexture());
      m_shader.setUniform( "u_brightness", m_data.brightness * easing );
      m_shader.setUniform( "u_saturation", m_data.saturation * easing );
      m_shader.setUniform( "u_contrast", m_data.contrast );
      m_shader.setUniform( "u_hueShift", m_data.hueShift );
      m_shader.setUniform( "u_gain", m_data.colorGain );

      m_outputTexture.clear();
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    ColorData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    inline static const std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform float u_brightness;      // 1.0 = normal
uniform float u_saturation;      // 1.0 = normal
uniform vec3 u_gain;             // RGB gain
uniform float u_contrast;        // 1.0 = normal
uniform float u_hueShift;        // radians

vec3 shiftHue(vec3 color, float angle) {
    const mat3 toYIQ = mat3( 0.299,  0.587,  0.114,
                             0.596, -0.275, -0.321,
                             0.212, -0.523,  0.311 );
    const mat3 toRGB = mat3( 1.0,  0.956,  0.621,
                            1.0, -0.272, -0.647,
                            1.0, -1.106,  1.703 );

    vec3 yiq = toYIQ * color;
    float hue = atan(yiq.z, yiq.y) + angle;
    float chroma = length(yiq.yz);
    yiq.y = chroma * cos(hue);
    yiq.z = chroma * sin(hue);
    return toRGB * yiq;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_texture, 0));
    vec4 color = texture2D(u_texture, uv);

    // Brightness
    color.rgb *= u_brightness;

    // Contrast
    color.rgb = (color.rgb - 0.5) * u_contrast + 0.5;

    // Gain
    color.rgb *= u_gain;

    // Saturation
    float gray = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    color.rgb = mix(vec3(gray), color.rgb, u_saturation);

    // Hue shift
    color.rgb = shiftHue(color.rgb, u_hueShift);

    gl_FragColor = color;
})";
  };
}