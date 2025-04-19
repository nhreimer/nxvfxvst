#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class DualKawaseBlurShader final : public IShader
  {

    struct DKBlurData_t
    {
      bool isActive { true };
      int passes = 4; // user-adjustable
      float offset = 1.0f;

      // 1.0	Subtle dreaminess
      // 2.0+	Cinematic glow
      // 3.0+	Pulse with the beat!
      float bloomGain = 1.0f;

      // 1.0	Natural balance
      // 1.5+	Lift post-blur darkening
      // 2.0+	Glowing nebula core vibes
      float brightness = 1.0f;

      // u_mixFactor = 0.0 → No bloom
      // u_mixFactor = 1.0 → Full bloom overlay
      // u_mixFactor > 1.0 → Overdriven glow
      float mixFactor = 1.0f;
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
      return
     {
          { "type", SerialHelper::serializeEnum( getType() ) },
          { "passes", m_data.passes },
          { "offset", m_data.offset },
          { "bloomGain", m_data.bloomGain },
          { "brightness", m_data.brightness },
          { "mixFactor", m_data.mixFactor },
          { "isActive", m_data.isActive },
          { "easing", m_easing.serialize() },
          { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", false);
        m_data.passes = j.value("passes", 4);
        m_data.offset = j.value("offset", 0.0f);
        m_data.bloomGain = j.value("bloomGain", 1.0f);
        m_data.brightness = j.value("brightness", 1.0f);
        m_data.mixFactor = j.value("mixFactor", 1.0f);
        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    // identify type for easier loading
    E_ShaderType getType() const override { return E_ShaderType::E_BlurShader; }

    void update( const sf::Time& deltaTime ) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Dual-Kawase Blur" ) )
      {
        ImGui::Checkbox( "DK Blur Active##1", &m_data.isActive );

        ImGui::SliderInt("Blur Passes", &m_data.passes, 1, 10);
        ImGui::SliderFloat("Blur Offset", &m_data.offset, 0.5f, 4.0f);
        ImGui::SliderFloat("Bloom Gain", &m_data.bloomGain, 0.1f, 5.0f);
        ImGui::SliderFloat("Blur Brightness", &m_data.brightness, 0.1f, 3.0f);
        ImGui::SliderFloat("Mix Factor", &m_data.mixFactor, 0.0f, 2.0f);

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
    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture)
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

      for (int i = 0; i < m_data.passes; ++i)
      {
        dst->clear();

        m_shader.setUniform("u_texture", src->getTexture());
        m_shader.setUniform("u_texelSize", sf::Glsl::Vec2(1.f / inputTexture.getSize().x, 1.f / inputTexture.getSize().y));
        m_shader.setUniform("u_offset", m_data.offset + i); // optional increase per pass
        m_shader.setUniform("u_bloomGain", m_data.bloomGain);     // user/MIDI-driven
        m_shader.setUniform("u_brightness", m_data.brightness);   // compensate blur


        dst->draw(sf::Sprite(src->getTexture()), &m_shader);
        dst->display();

        std::swap(src, dst); // ping-pong
      }

      // Composite final bloom with original
      m_compositeTexture.clear();
      m_compositeShader.setUniform("u_scene", inputTexture.getTexture());
      m_compositeShader.setUniform("u_bloom", src->getTexture());
      m_compositeShader.setUniform("u_mixFactor", m_data.mixFactor);

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