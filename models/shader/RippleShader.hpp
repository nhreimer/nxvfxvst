#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{
  class RippleShader final : public IShader
  {

#define RIPPLE_SHADER_PARAMS(X)                                                                   \
X(rippleCenterX, float, 0.5f, 0.f, 1.f,  "Horizontal origin of ripple (0.0 = left, 1.0 = right)") \
X(rippleCenterY, float, 0.5f, 0.f, 1.f,  "Vertical origin of ripple (0.0 = top, 1.0 = bottom)")   \
X(amplitude,     float, 0.05f, 0.f, 1.f, "[CALC] Ripple strength, typically eased")               \
X(frequency,     float, 10.f,  1.f, 100.f,"Wave density across the screen")                       \
X(speed,         float, 0.f,   0.f, 50.f,"Wave movement speed over time")


    struct RippleData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(RIPPLE_SHADER_PARAMS)
    };

    enum class E_RippleParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(RIPPLE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_RippleParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(RIPPLE_SHADER_PARAMS)
    };

  public:

    explicit RippleShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load ripple fragment shader" );
      }
    }

    ~RippleShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(RIPPLE_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(RIPPLE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_RippleShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Ripple" ) )
      {
        ImGui::Checkbox( "Ripple Active##1", &m_data.isActive );

        const float oldCenterX = m_data.rippleCenterX;
        const float oldCenterY = m_data.rippleCenterY;

        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        RIPPLE_SHADER_PARAMS(X_SHADER_IMGUI);

        if ( oldCenterX != m_data.rippleCenterX || oldCenterY != m_data.rippleCenterY )
        {
          const sf::Vector2f calibrated { m_data.rippleCenterX * static_cast< float >(m_globalInfo.windowSize.x),
                                          m_data.rippleCenterY * static_cast< float >(m_globalInfo.windowSize.y) };
          m_timedCursor.setPosition( calibrated );
        }

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture & applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize ripple texture" );
        }
      }

      constexpr float baseAmplitude = 0.005f;
      constexpr float maxPulseAmplitude = 0.03f;

      const float eased = m_easing.getEasing();
      m_data.amplitude = baseAmplitude + eased * maxPulseAmplitude;

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform( "resolution", sf::Vector2f( inputTexture.getSize() ) );
      m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );

      m_shader.setUniform( "rippleCenter", sf::Vector2f( m_data.rippleCenterX, m_data.rippleCenterY) );
      m_shader.setUniform( "amplitude", m_data.amplitude );     // 0.0f – 0.05f
      m_shader.setUniform( "frequency", m_data.frequency );     // 10.0f – 50.0f
      m_shader.setUniform( "speed", m_data.speed );             // 0.0f – 10.0f

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    RippleData_t m_data;

    sf::Clock m_clock;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    TimedCursorPosition m_timedCursor;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;

uniform vec2 rippleCenter;
uniform float amplitude;   // base amplitude
uniform float frequency;
uniform float speed;

uniform float intensity;   // added: easing-controlled value [0..1]

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec2 delta = uv - rippleCenter;
    float dist = length(delta);

    // Ripple effect, scaled by intensity
    float ripple = sin(dist * frequency - time * speed) * amplitude;// * intensity;

    vec2 dir = normalize(delta);
    uv += dir * ripple;

    gl_FragColor = texture2D(texture, uv);
})";
  };

}