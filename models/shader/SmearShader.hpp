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
    int sampleCount { 32 };                            // more = smoother, but slower

    float jitterAmount { 0.0f };
    float brightnessBoost{ 1.0f };
    float brightnessPulse{ 1.f };
    float falloffPower { 1.0f };

    float wiggleAmplitude { 0.4f };    // in radians
    float wiggleFrequency { 5.0f };    // Hz

    float feedbackFade = 0.05f; // 0 = no fade, 1 = instant clear
    sf::BlendMode feedbackBlendMode { sf::BlendAdd };

    float feedbackRotation = 0.f;  // degrees
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
          { "type", SerialHelper::serializeEnum( getType() ) },
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
          { "feedbackFade", m_data.feedbackFade },
          { "feedbackBlend", SerialHelper::convertBlendModeToString( m_data.feedbackBlendMode ) },
          { "easing", m_easing.serialize() },
          { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }
    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.at( "isActive" ).get<bool>();
        m_data.intensity = j.at( "intensity" ).get<float>();
        m_data.length = j.at( "length" ).get<float>();
        m_data.tint = SerialHelper::convertColorFromJson( j.at( "tint" ), sf::Color::White );
        m_data.sampleCount = j.at( "sampleCount" ).get<int>();
        m_data.jitterAmount = j.at( "jitterAmount" ).get<float>();
        m_data.brightnessBoost = j.at( "brightnessBoost" ).get<float>();
        m_data.brightnessPulse = j.at( "brightnessPulse" ).get<float>();
        m_data.wiggleAmplitude = j.at( "wiggleAmplitude" ).get<float>();
        m_data.wiggleFrequency = j.at( "wiggleFrequency" ).get<float>();
        m_data.directionAngleInRadians = j.at( "directionAngleInRadians" ).get<float>();
        m_data.falloffPower = j.at( "falloffPower" ).get<float>();
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_SmearShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      {
        m_easing.trigger();
      }
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

        // after .5 - .7, there's a rolloff that occurs
        ImGui::SliderFloat("Jitter Amount", &m_data.jitterAmount, 0.f, 0.8f);
        ImGui::SliderFloat("Falloff Power", &m_data.falloffPower, 0.5f, 4.f);

        ImGui::SliderFloat("Feedback Fade", &m_data.feedbackFade, 0.0f, 1.0f);

        ImVec4 color = m_data.tint;
        if ( ImGui::ColorPicker4( "Particle Fill##1",
                                  reinterpret_cast< float * >( &color ),
                                  ImGuiColorEditFlags_AlphaBar,
                                  nullptr ) )
        {
          m_data.tint = color;
        }

        ImGui::Separator();
        MenuHelper::drawBlendOptions( m_data.feedbackBlendMode );

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
        if ( !m_outputTexture.resize( inputTexture.getSize() ) ||
             !m_feedbackTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize smear texture" );
        }
        else
        {
          m_feedbackTexture.clear( sf::Color::Black );
          m_feedbackTexture.display();
          m_feedbackFadeShape.setSize( { static_cast< float >(inputTexture.getSize().x),
                                            static_cast< float >(inputTexture.getSize().y) } );
        }
      }

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
           static_cast< float >(m_data.tint.r) / 255.f,
           static_cast< float >(m_data.tint.g) / 255.f,
           static_cast< float >(m_data.tint.b) / 255.f
       );

      m_shader.setUniform("smearTint", tintVec);

      // 1. Use the previous feedback frame as input
      const sf::Sprite feedbackSprite( m_feedbackTexture.getTexture() );

      // 2. Apply the smear shader TO the feedback (draw into m_outputTexture)
      m_outputTexture.clear();
      m_outputTexture.draw(feedbackSprite, &m_shader);
      m_outputTexture.display();

      // 3. Fade feedback with a semi-transparent black quad to prevent infinite trails
      m_feedbackFadeShape.setFillColor(
        sf::Color(0, 0, 0,
                    static_cast< uint8_t >( 255 * m_data.feedbackFade ) ) );

      m_feedbackTexture.draw(m_feedbackFadeShape, sf::BlendAlpha);

      // 4. Add current smeared frame into feedback buffer
      const sf::Sprite smearedFrame(m_outputTexture.getTexture());
      m_feedbackTexture.draw(smearedFrame, m_data.feedbackBlendMode);

      // 5. Display feedback buffer
      m_feedbackTexture.display();

      // 6. Output the feedback as final result
      return m_feedbackTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    SmearData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;

    sf::RenderTexture m_outputTexture;
    sf::RenderTexture m_feedbackTexture;

    TimeEasing m_easing;
    MidiNoteControl m_midiNoteControl;

    sf::RectangleShape m_feedbackFadeShape;

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