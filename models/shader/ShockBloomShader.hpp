#pragma once

#include "models/data/PipelineContext.hpp"
#include "models/IShader.hpp"

namespace nx
{

  class ShockBloomShader final : public IShader
  {

#define SHOCK_BLOOM_SHADER_PARAMS(X)                                                                         \
X(center,            sf::Vector2f, sf::Vector2f(0.5f, 0.5f), 0.f, 0.f, "Center of the ring (UV)", false)     \
X(color,             sf::Glsl::Vec3, sf::Glsl::Vec3(1.f, 1.f, 1.f), 0.f, 0.f, "Shock ring RGB color", false) \
X(maxRadius,         float, 0.6f,   0.1f, 1.5f,  "Maximum radius of the ring", true)                        \
X(thickness,         float, 0.05f,  0.01f, 0.2f, "Ring thickness (falloff)", true)                          \
X(intensity,         float, 2.0f,   0.0f, 5.0f,  "Glow strength of the ring", true)                         \
X(easingMultiplier,  float, 1.0f,   0.0f, 5.0f,  "How strongly the easing drives visibility", true)         \
X(innerTransparency, float, 1.0f, 0.f, 1.f, "Transparency multiplier for the center of the ring", true)     \
X(mixFactor,         float, 1.0f, 0.f, 1.f, "Mix between original and effects result", true)                \
X(BlendInput,        sf::BlendMode, sf::BlendAdd, 0.f, 0.f, nullptr, false )

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

    explicit ShockBloomShader(PipelineContext& context)
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load shock bloom fragment shader" );
      }
      else
      {
        LOG_INFO( "Shock bloom shader loaded successfully" );
      }

      EXPAND_SHADER_VST_BINDINGS(SHOCK_BLOOM_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~ShockBloomShader() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
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

        const float oldCenterX = m_data.center.first.x;
        const float oldCenterY = m_data.center.first.y;

        EXPAND_SHADER_IMGUI(SHOCK_BLOOM_SHADER_PARAMS, m_data)

        if ( oldCenterX != m_data.center.first.x || oldCenterY != m_data.center.first.y )
        {
          const sf::Vector2f calibrated { m_data.center.first.x * static_cast< float >(m_ctx.globalInfo.windowSize.x),
                                          m_data.center.first.y * static_cast< float >(m_ctx.globalInfo.windowSize.y) };
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
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize shock bloom texture" );
        }
        else
        {
          LOG_INFO( "resized shock bloom texture" );
        }
      }

      const float easing = m_easing.getEasing();
      const float radius = m_data.maxRadius.first * easing;
      const float alpha = easing * m_data.easingMultiplier.first;

      // Update uniforms
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));
      m_shader.setUniform("center", m_data.center.first);
      m_shader.setUniform("radius", radius);
      m_shader.setUniform("thickness", m_data.thickness.first);
      m_shader.setUniform("color", m_data.color.first);
      m_shader.setUniform("intensity", m_data.intensity.first * alpha);
      m_shader.setUniform("innerTransparency", m_data.innerTransparency.first);

      // Fullscreen quad
      sf::RectangleShape fullscreen(sf::Vector2f(inputTexture.getSize()));
      fullscreen.setFillColor(sf::Color::White);

      m_outputTexture.clear(sf::Color::Transparent);
      m_outputTexture.draw(fullscreen, &m_shader);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture, m_outputTexture, m_data.mixFactor.first );
    }

  private:

    PipelineContext& m_ctx;
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
uniform float innerTransparency;

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