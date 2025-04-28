#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class StrobeShader final : public IShader
  {

#define STROBE_SHADER_PARAMS(X)                                                                      \
X(flashAmount,  float, 20.f, 1.f, 100.f, "Speed of strobe pulses (Hz)", true)                        \
X(flashColor,   sf::Color, sf::Color::White, 0.f, 0.f, "Flash color applied during strobe", true)    \
X(mixFactor,    float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

    struct StrobeData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(STROBE_SHADER_PARAMS)
    };

    enum class E_StrobeParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(STROBE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_StrobeParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(STROBE_SHADER_PARAMS)
    };

  public:
    explicit StrobeShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load strobe fragment shader" );
      }
      else
      {
        LOG_INFO( "Strobe shader loaded successfully" );
      }

      EXPAND_SHADER_VST_BINDINGS(STROBE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~StrobeShader() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(STROBE_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(STROBE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_StrobeShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Strobe Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        STROBE_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture &applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize strobe texture" );
        }
        else
        {
          LOG_INFO( "resized strobe texture" );
        }
      }

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform("flashAmount", m_data.flashAmount * m_easing.getEasing()); // or assigned easing
      m_shader.setUniform("flashColor", sf::Glsl::Vec4(m_data.flashColor));

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                              m_outputTexture,
                              m_data.mixFactor );
    }

  private:
    PipelineContext& m_ctx;
    StrobeData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float flashAmount;     // 0.0 = normal scene, 1.0 = full flash
uniform vec4 flashColor;       // user-defined color

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(texture, 0));
    vec4 base = texture2D(texture, uv);

    // Blend ENTIRE scene toward flashColor based on flashAmount
    vec3 finalColor = mix(base.rgb, flashColor.rgb, flashAmount);

    gl_FragColor = vec4(finalColor, flashColor.a);
})";
  };
}