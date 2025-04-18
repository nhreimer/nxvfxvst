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
    };

  public:

    explicit DualKawaseBlurShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load blur fragment shader" );
      }
      else
      {
        LOG_INFO( "loaded blur shader" );
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
          { "isActive", m_data.isActive }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", false);
        // m_data.sigma = j.value("sigma", 7.f);
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
             !m_pongTexture.resize( inputTexture.getSize() ) )
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

      return *src; // final output
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::Shader m_shader;

    // ping-pong texture strategy for n passes
    sf::RenderTexture m_pingTexture;
    sf::RenderTexture m_pongTexture;

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

  };
}