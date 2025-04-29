#pragma once

namespace nx
{

  class SmearShader final : public IShader
  {
#define SMEAR_SHADER_PARAMS(X)                                                                       \
X(directionAngleInRadians, float, 0.f,  -NX_PI, NX_PI,  "Angle of smear direction (in radians)", true)     \
X(length,                 float, 0.2f,  0.f,  1.f,     "Length of smear trail (0 = off, 1 = full)", true)  \
X(intensity,              float, 0.5f,  0.f,  1.f,     "Blend amount with previous frame", true)           \
X(tint,                   sf::Color, sf::Color(255,255,255), 0.f, 0.f, "Optional color overlay", false)     \
X(sampleCount,            int,   32,    1,   128,      "Number of smear samples (quality vs speed)", true) \
X(jitterAmount,           float, 0.f,   0.f,  1.f,     "Randomness in smear direction per sample", true)   \
X(brightnessBoost,        float, 1.f,   0.f,  10.f,    "Overall brightness multiplier", true)              \
X(brightnessPulse,        float, 1.f,   0.f,  5.f,     "Brightness wave amount during pulses", true)       \
X(falloffPower,           float, 1.f,   0.1f, 10.f,    "Exponential falloff on smear fade", true)          \
X(wiggleAmplitude,        float, 0.f,   0.f,  NX_PI,   "Wiggle amount (radians) per sample", true)         \
X(wiggleFrequency,        float, 0.f,   0.f,  50.f,    "Wiggle speed (Hz)", true)                          \
X(feedbackFade,           float, 0.05f, 0.f,  1.f,     "Fadeout amount for feedback trail", true)          \
X(feedbackBlendMode,      sf::BlendMode, sf::BlendAdd, 0, 0, "Blend mode used for feedback drawing", false) \
X(feedbackRotation,       float, 0.f,   -360.f, 360.f, "Rotational offset added to feedback frame", true)  \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct SmearData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(SMEAR_SHADER_PARAMS)
    };

    enum class E_SmearParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(SMEAR_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_SmearParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(SMEAR_SHADER_PARAMS)
    };

  public:

    explicit SmearShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load smear fragment shader" );
      }
      else
      {
        LOG_INFO( "Smear fragment shader loaded" );
      }

      EXPAND_SHADER_VST_BINDINGS(SMEAR_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~SmearShader() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(SMEAR_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(SMEAR_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
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
      if ( ImGui::TreeNode( "Smear Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        EXPAND_SHADER_IMGUI(SMEAR_SHADER_PARAMS, m_data)

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
      //return m_feedbackTexture;
      return m_blender.applyShader( inputTexture,
                              m_feedbackTexture,
                                    m_data.mixFactor );
    }

  private:
    PipelineContext& m_ctx;

    SmearData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;

    sf::RenderTexture m_outputTexture;
    sf::RenderTexture m_feedbackTexture;

    BlenderShader m_blender;
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