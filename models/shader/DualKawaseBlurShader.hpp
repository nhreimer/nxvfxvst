#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class DualKawaseBlurShader final : public IShader
  {

    // The BlenderShader for this one is already built in. it was the prototype example.
#define DUALKAWASE_SHADER_PARAMS(X)                                                           \
X(passes,      int,   4,     1,    10,    "Number of downsample/upsample passes")             \
X(offset,      float, 1.0f,  0.0f, 10.f,  "Kernel offset per pass")                           \
X(bloomGain,   float, 1.0f,  0.0f, 10.f,  "Gain applied to bloom texture before blending")    \
X(brightness,  float, 1.0f,  0.0f, 3.f,   "Brightness boost for final output")                \
X(mixFactor,   float, 1.0f,  0.0f, 1.f,   "Blend factor between base and blurred result")

    struct DKBlurData_t
    {
      bool isActive = true;
      EXPAND_SHADER_PARAMS_FOR_STRUCT(DUALKAWASE_SHADER_PARAMS)
    };

    enum class E_DualKawaseParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(DUALKAWASE_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_DualKawaseParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(DUALKAWASE_SHADER_PARAMS)
    };

  public:

    explicit DualKawaseBlurShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) ||
           !m_compositeShader.loadFromMemory( m_compositeFragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load dk blur fragment shader" );
      }
      else
      {
        LOG_INFO( "loaded dk blur shader" );
      }
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum(getType());
      EXPAND_SHADER_PARAMS_TO_JSON(DUALKAWASE_SHADER_PARAMS)
      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(DUALKAWASE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    // identify type for easier loading
    E_ShaderType getType() const override { return E_ShaderType::E_DualKawaseBlurShader; }

    void update( const sf::Time& deltaTime ) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Dual-Kawase Blur Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        DUALKAWASE_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture) override
    {
      if ( m_pingTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_pingTexture.resize( inputTexture.getSize() ) ||
             !m_pongTexture.resize( inputTexture.getSize() ) ||
             !m_compositeTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "Failed to resize DK Blur Texture(s) texture" );
        }
        else
        {
          LOG_INFO( "Resized DK Blur Textures" );
        }
      }

      sf::RenderTexture* src = &m_pingTexture;
      sf::RenderTexture* dst = &m_pongTexture;

      m_pingTexture.clear();
      m_pingTexture.draw(sf::Sprite(inputTexture.getTexture()));
      m_pingTexture.display();

      const auto easing = m_easing.getEasing();

      for (int i = 0; i < m_data.passes; ++i)
      {
        dst->clear();

        m_shader.setUniform("u_texture", src->getTexture());
        m_shader.setUniform("u_texelSize", sf::Glsl::Vec2(1.f / inputTexture.getSize().x, 1.f / inputTexture.getSize().y));
        m_shader.setUniform("u_offset", m_data.offset + i); // optional increase per pass
        m_shader.setUniform("u_bloomGain", m_data.bloomGain * easing);     // user/MIDI-driven
        m_shader.setUniform("u_brightness", m_data.brightness * easing);   // compensate blur


        dst->draw(sf::Sprite(src->getTexture()), &m_shader);
        dst->display();

        std::swap(src, dst); // ping-pong
      }

      // Composite final bloom with original
      m_compositeTexture.clear();
      m_compositeShader.setUniform("u_scene", inputTexture.getTexture());
      m_compositeShader.setUniform("u_bloom", src->getTexture());
      m_compositeShader.setUniform("u_mixFactor", m_data.mixFactor * easing);

      m_compositeTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_compositeShader );
      m_compositeTexture.display();

      return m_compositeTexture;
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::Shader m_shader;
    sf::Shader m_compositeShader;

    // ping-pong texture strategy for n passes + composite for mixing
    sf::RenderTexture m_pingTexture;
    sf::RenderTexture m_pongTexture;
    sf::RenderTexture m_compositeTexture;

    DKBlurData_t m_data;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D u_texture;
uniform vec2 u_texelSize;
uniform float u_offset;
uniform float u_bloomGain;     // global gain
uniform float u_brightness;    // base boost

void main() {
    vec2 uv = gl_FragCoord.xy * u_texelSize;

    vec3 color = texture2D(u_texture, uv).rgb * 4.0;
    color += texture2D(u_texture, uv + u_texelSize * vec2( u_offset,  u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2(-u_offset,  u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2( u_offset, -u_offset)).rgb;
    color += texture2D(u_texture, uv + u_texelSize * vec2(-u_offset, -u_offset)).rgb;

    color = color / 8.0;

    // BOOST!
    color *= u_brightness;
    color *= u_bloomGain;

    gl_FragColor = vec4(color, 1.0);
})";

    inline static const std::string m_compositeFragmentShader = R"(uniform sampler2D u_scene;     // original render
uniform sampler2D u_bloom;     // blurred bloom texture
uniform float u_mixFactor;     // 0.0 = off, 1.0 = full blend, >1.0 = glow boost

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(u_scene, 0));

    vec3 sceneColor = texture2D(u_scene, uv).rgb;
    vec3 bloomColor = texture2D(u_bloom, uv).rgb;

    // Blend bloom back into the original
    vec3 finalColor = mix(sceneColor, sceneColor + bloomColor, u_mixFactor);

    gl_FragColor = vec4(finalColor, 1.0);
}
)";

  };
}