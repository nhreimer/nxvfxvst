#pragma once

namespace nx
{

  struct SmearData_t
  {
    bool isActive { true };
    float directionAngleInRadians { 0.f };
    float length { 0.2f };                             // 0.0–1.0
    float intensity { 0.5f };                          // blend amount
    sf::Color tint { 255, 255, 255 };   // optional colorization
    int sampleCount { 12 };                            // more = smoother, but slower

    float jitterAmount { 0.0f };
    float brightnessBoost{ 1.0f };
    float brightnessPulse{ 1.f };
    float falloffPower { 1.0f };

    float wiggleAmplitude { 0.4f };    // in radians
    float wiggleFrequency { 5.0f };    // Hz
  };

  class SmearShader final : public IShader
  {
  public:

    explicit SmearShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load smear fragment shader" );
      }
    }

    ~SmearShader() override = default;

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
    {
          { "type", SerialHelper::convertShaderTypeToString( getType() ) },
          { "isActive", m_data.isActive },
          { "directionAngleInRadians", m_data.directionAngleInRadians },
          { "length", m_data.length },
          { "tint", SerialHelper::convertColorToJson( m_data.tint )  },
          { "sampleCount", m_data.sampleCount },
          { "jitterAmount", m_data.jitterAmount },
          { "brightnessBoost", m_data.brightnessBoost },
          { "brightnessPulse", m_data.brightnessPulse },
          { "falloffPower", m_data.falloffPower },
          { "wiggleAmplitude", m_data.wiggleAmplitude },
          { "wiggleFrequency", m_data.wiggleFrequency },
          { "easing", m_easing.serialize() },
          { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }
    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_SmearShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Smear" ) )
      {
        ImGui::Checkbox("Active", &m_data.isActive);
        ImGui::SliderFloat("Smear Length", &m_data.length, 0.f, 1.f);
        ImGui::SliderFloat("Smear Intensity", &m_data.intensity, 0.f, 2.f);
        ImGui::SliderInt("Samples", &m_data.sampleCount, 1, 128);

        ImGui::SliderAngle("Smear Direction", &m_data.directionAngleInRadians, -180.f, 180.f);
        ImGui::SliderFloat("Wiggle Amplitude", &m_data.wiggleAmplitude, 0.f, 2.0f); // radians
        ImGui::SliderFloat("Wiggle Frequency", &m_data.wiggleFrequency, 0.f, 20.0f); // Hz

        ImGui::SliderFloat("Brightness Boost", &m_data.brightnessBoost, 1.f, 10.f);
        ImGui::SliderFloat("Jitter Amount", &m_data.jitterAmount, 0.f, 0.3f);
        ImGui::SliderFloat("Falloff Power", &m_data.falloffPower, 0.5f, 4.f);

        ImVec4 color = m_data.tint;
        if ( ImGui::ColorPicker4( "Particle Fill##1",
                                  reinterpret_cast< float * >( &color ),
                                  ImGuiColorEditFlags_AlphaBar,
                                  nullptr ) )
        {
          m_data.tint = color;
        }

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture & applyShader(const sf::RenderTexture &inputTexture) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize smear texture" );
        }
      }

      m_outputTexture.clear( sf::Color::Transparent );

      const float easing = m_easing.getEasing();

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize() ) );
      m_shader.setUniform("smearLength", m_data.length);
      m_shader.setUniform("smearIntensity", m_data.intensity);
      m_shader.setUniform("sampleCount", m_data.sampleCount);

      m_shader.setUniform("time", m_clock.getElapsedTime().asSeconds());
      m_shader.setUniform("jitterAmount", m_data.jitterAmount);         // 0.0–0.2
      m_shader.setUniform("brightnessBoost", m_data.brightnessBoost);   // 1.0–3.0
      m_shader.setUniform("pulseValue", easing);
      m_shader.setUniform("falloffPower", m_data.falloffPower);         // e.g. 1.0 = linear, >1 = tighter fade
      m_shader.setUniform("brightnessPulse", easing);

      m_shader.setUniform("directionAngle", m_data.directionAngleInRadians);
      m_shader.setUniform("wiggleAmplitude", m_data.wiggleAmplitude);
      m_shader.setUniform("wiggleFrequency", m_data.wiggleFrequency);

       const auto tintVec = sf::Glsl::Vec3(
           m_data.tint.r / 255.f,
           m_data.tint.g / 255.f,
           m_data.tint.b / 255.f
       );

      m_shader.setUniform("smearTint", tintVec);

      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    SmearData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;
    TimeEasing m_easing;
    MidiNoteControl m_midiNoteControl;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;

uniform float smearLength;
uniform float smearIntensity;
uniform vec3 smearTint;
uniform int sampleCount;

uniform float jitterAmount;
uniform float brightnessBoost;
uniform float brightnessPulse;
uniform float pulseValue;
uniform float falloffPower;

uniform float directionAngle;
uniform float wiggleAmplitude;
uniform float wiggleFrequency;
uniform float time;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec4 baseColor = texture2D(texture, uv);
    vec4 smearColor = vec4(0.0);

    //vec2 dir = normalize(smearDirection);

    // wiggle, wiggle, baby!
    float wiggle = sin(time * wiggleFrequency) * wiggleAmplitude;
    float angle = directionAngle + wiggle;
    vec2 dir = vec2(cos(angle), sin(angle));

    for (int i = 1; i <= sampleCount; ++i) {
        float t = float(i) / float(sampleCount);
        float falloff = pow(1.0 - t, falloffPower); // falloff control

        // Jitter
        float angle = hash(t + time) * 6.28318; // random angle
        vec2 jitter = vec2(cos(angle), sin(angle)) * jitterAmount * t;

        vec2 offset = -dir * smearLength * t + jitter;
        vec4 sample = texture2D(texture, uv + offset);
        sample.rgb *= smearTint * falloff;
        smearColor += sample;
    }

    smearColor /= float(sampleCount);

    // Intensity blend
    vec4 blended = mix(baseColor, smearColor, clamp(smearIntensity + pulseValue, 0.0, 1.0));

    // Brightness boost (final optional pop)
    //blended.rgb *= brightnessBoost;
    blended.rgb *= brightnessBoost + brightnessPulse;

    gl_FragColor = blended;
})";

  };

}