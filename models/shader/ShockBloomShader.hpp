#pragma once
#include "models/IShader.hpp"
#include "models/shader/BlenderShader.hpp"

namespace nx
{

  class ShockBloomShader final : public IShader
  {

#define SHOCK_BLOOM_SHADER_PARAMS(X)                                                                  \
X(center,            sf::Vector2f, sf::Vector2f(0.5f, 0.5f), 0.f, 0.f, "Center of the ring (UV)")     \
X(color,             sf::Glsl::Vec3, sf::Glsl::Vec3(1.f, 1.f, 1.f), 0.f, 0.f, "Shock ring RGB color") \
X(maxRadius,         float, 0.6f,   0.1f, 1.5f,  "Maximum radius of the ring")                        \
X(thickness,         float, 0.05f,  0.01f, 0.2f, "Ring thickness (falloff)")                          \
X(intensity,         float, 2.0f,   0.0f, 5.0f,  "Glow strength of the ring")                         \
X(BlendInput,        sf::BlendMode, sf::BlendAdd, 0.f, 0.f, "Blends the effect over the input" )      \
X(easingMultiplier,  float, 1.0f,   0.0f, 5.0f,  "How strongly the easing drives visibility")         \
X(innerTransparency, float, 1.0f, 0.f, 1.f, "Transparency multiplier for the center of the ring")     \
X(mixFactor,         float, 1.0f, 0.f, 1.f, "Mix between original and effects result")                \
X(BlendMix,          sf::BlendMode, sf::BlendAdd, 0.f, 0.f, "Blends the mixed effect" )


    struct ShockBloomData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(SHOCK_BLOOM_SHADER_PARAMS)
    };

    enum class E_ShockBloomParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(SHOCK_BLOOM_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_ShockBloomParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(SHOCK_BLOOM_SHADER_PARAMS)
    };

  public:

    explicit ShockBloomShader(const GlobalInfo_t& globalInfo)
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load shock bloom fragment shader" );
      }
      else
      {
        LOG_INFO( "Shock bloom shader loaded successfully" );

        m_easing.setEasingType( E_TimeEasingType::E_Linear );
      }
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(SHOCK_BLOOM_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(SHOCK_BLOOM_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_ShaderType::E_ShockBloomShader; }

    void update( const sf::Time& deltaTime ) override
    {}

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Shock Bloom Options" ) )
      {
        ImGui::Checkbox( "Strobe Active##1", &m_data.isActive );

        const float oldCenterX = m_data.center.x;
        const float oldCenterY = m_data.center.y;

        auto& STRUCT_REF = m_data;
        SHOCK_BLOOM_SHADER_PARAMS(X_SHADER_IMGUI);

        if ( oldCenterX != m_data.center.x || oldCenterY != m_data.center.y )
        {
          const sf::Vector2f calibrated { m_data.center.x * static_cast< float >(m_globalInfo.windowSize.x),
                                          m_data.center.y * static_cast< float >(m_globalInfo.windowSize.y) };
          m_timedCursor.setPosition( calibrated );
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
    sf::RenderTexture& applyShader( const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize shock bloom texture" );
        }
        else
        {
          LOG_INFO( "resized shock bloom texture" );
        }
      }

      const float easing = m_easing.getEasing();
      const float radius = m_data.maxRadius * easing;
      const float alpha = easing * m_data.easingMultiplier;

      // Update uniforms
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));
      m_shader.setUniform("center", m_data.center);
      m_shader.setUniform("radius", radius);
      m_shader.setUniform("thickness", m_data.thickness);
      m_shader.setUniform("color", m_data.color);
      m_shader.setUniform("intensity", m_data.intensity * alpha);
      m_shader.setUniform("innerTransparency", m_data.innerTransparency);

      // Draw with optional blend mode
      sf::RenderStates states;
      states.shader = &m_shader;
      states.blendMode = m_data.BlendInput;

      // Fullscreen quad
      sf::RectangleShape fullscreen(sf::Vector2f(inputTexture.getSize()));
      fullscreen.setFillColor(sf::Color::White);

      m_outputTexture.clear(sf::Color::Transparent);
      m_outputTexture.draw(fullscreen, states);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture, m_outputTexture, m_data.mixFactor, m_data.BlendMix );
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    ShockBloomData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    TimedCursorPosition m_timedCursor;

    inline static const std::string m_fragmentShader = R"(uniform vec2 resolution;
uniform vec2 center;
uniform float radius;
uniform float thickness;
uniform vec3 color;
uniform float intensity;
uniform float innerTransparency; // new!

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    float dist = length(uv - center);

    // Ring falloff
    float edgeFade = smoothstep(radius, radius - thickness, dist) *
                     smoothstep(radius + thickness, radius, dist);

    // Inner transparency falloff
    float innerCutout = smoothstep(0.0, radius - thickness * 0.5, dist); // transparent inside
    float ringShape = edgeFade * innerCutout;

    vec3 glow = color * ringShape * intensity;

    gl_FragColor = vec4(glow, ringShape * innerTransparency); // final transparency applied
})";

  };

}