#pragma once

namespace nx
{
  class TransformShader final : public IShader
  {
#define TRANSFORM_SHADER_PARAMS(X)                                                                 \
X(rotationDegrees, float, 0.f,   -360.f, 360.f, "Rotation applied to the screen", true)            \
X(shift,           sf::Vector2f, sf::Vector2f(0.f, 0.f), 0.f, 0.f, "Screen offset (X, Y)", false)  \
X(flipX,           bool, false,  0, 0, "Horizontal flip toggle", false)                            \
X(flipY,           bool, false,  0, 0, "Vertical flip toggle", false)                              \
X(scale,           float, 1.f,   0.1f, 5.f, "Uniform scale factor for zoom or shrink", true)       \
X(mixFactor,       float, 1.0f,  0.f,  1.f, "Mix between original and effects result", true)

    struct TransformData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(TRANSFORM_SHADER_PARAMS)
    };

    enum class E_TransformParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(TRANSFORM_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_TransformParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(TRANSFORM_SHADER_PARAMS)
    };

  public:

    explicit TransformShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load transform fragment shader" );
      }
      else
      {
        LOG_DEBUG( "Transform fragment shader loaded successfully" );
        m_easing.setEasingType( E_TimeEasingType::E_Disabled );
      }

      EXPAND_SHADER_VST_BINDINGS(TRANSFORM_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~TransformShader() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(TRANSFORM_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(TRANSFORM_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_TransformShader; }

    void update(const sf::Time &deltaTime) override {}

    void trigger(const Midi_t &midi) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Transform Options") )
      {

        const float offsetX = m_data.shift.first.x;
        const float offsetY = m_data.shift.first.y;

        EXPAND_SHADER_IMGUI(TRANSFORM_SHADER_PARAMS, m_data)

        if ( offsetX != m_data.shift.first.x || offsetY != m_data.shift.first.y )
        {
          const sf::Vector2f calibrated
          {
            ( m_data.shift.first.x + 0.5f ) * static_cast< float >( m_ctx.globalInfo.windowSize.x ),
            ( m_data.shift.first.y + 0.5f ) * static_cast< float >( m_ctx.globalInfo.windowSize.y )
          };

          m_timedCursorShift.setPosition( calibrated );
        }

        ImGui::SeparatorText( "Easings" );
        m_easing.drawMenu();

        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursorShift.hasExpired() )
        m_timedCursorShift.drawPosition();

      // if ( !m_timedCursorStretch.hasExpired() )
      //   m_timedCursorStretch.drawPosition();
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

      auto const easing = m_easing.getEasing();

      m_shader.setUniform("u_texture", inputTexture.getTexture());
      m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture.getSize() });
      m_shader.setUniform("u_offset", sf::Glsl::Vec2( m_data.shift.first ) );
      m_shader.setUniform("u_scale", m_data.scale.first * easing );

      m_shader.setUniform("u_rotation", sf::degrees(m_data.rotationDegrees.first).asRadians());
      m_shader.setUniform("u_flipX", m_data.flipX.first);
      m_shader.setUniform("u_flipY", m_data.flipY.first);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                              m_outputTexture,
                              m_data.mixFactor.first );
    }

  private:
    PipelineContext& m_ctx;

    TransformData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    TimedCursorPosition m_timedCursorShift;
    // TimedCursorPosition m_timedCursorStretch;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform vec2 u_offset;
uniform float u_scale;
uniform float u_rotation; // radians
uniform bool u_flipX;
uniform bool u_flipY;

void main() {
    // Pixel-space coordinates
    vec2 pixelCoord = gl_FragCoord.xy;

    // Flip in screen space (if needed)
    if (u_flipX) pixelCoord.x = u_resolution.x - pixelCoord.x;
    if (u_flipY) pixelCoord.y = u_resolution.y - pixelCoord.y;

    // Center of screen in pixels
    vec2 center = 0.5 * u_resolution;

    // Translate to origin, scale, rotate, then translate back
    vec2 pos = pixelCoord - center;
    pos /= u_scale;

    float cosA = cos(u_rotation);
    float sinA = sin(u_rotation);
    pos = mat2(cosA, -sinA, sinA, cosA) * pos;

    pos += center;

    // Apply offset in pixels (not UV!)
    pos += vec2(-u_offset.x * u_resolution.x, u_offset.y * u_resolution.y);

    // Convert back to normalized UVs
    vec2 uv = pos / u_resolution;

    gl_FragColor = texture2D(u_texture, uv);
})";

  };
}