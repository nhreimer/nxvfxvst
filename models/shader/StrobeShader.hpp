#pragma once
#include "shapes/MidiNoteControl.hpp"

namespace nx
{

  // TODO: SERIALIZE ALL PARTS OF THIS!
  struct StrobeData_t
  {
    bool isActive { true };
    float pulseSpeed { 20.f };
  };

  class StrobeShader final : public IShader
  {
  public:
    explicit StrobeShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load strobe fragment shader" );
      }
    }

    ~StrobeShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", SerialHelper::convertShaderTypeToString( getType() ) },
          { "isActive", m_data.isActive },
          { "midiTriggers", m_midiNoteControl.serialize() },
            { "easing", m_easing.serialize() }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_midiNoteControl.deserialize( j.at( "midiTriggers" ) );
      m_easing.deserialize( j.at( "easing" ) );
    }

    E_ShaderType getType() const override { return E_StrobeShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Strobe" ) )
      {
        ImGui::Checkbox( "Strobe Active##1", &m_data.isActive );
        ImGui::SliderFloat( "##Pulse Speed", &m_data.pulseSpeed, 0.f, 50.f, "Speed %0.2f" );

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
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize strobe texture" );
        }
      }

      const float intensity = m_easing.getEasing();

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform( "intensity", intensity );
      m_shader.setUniform( "pulseSpeed", m_data.pulseSpeed );

      // const auto targetColor = sf::Glsl::Vec3( m_data.flashColor.r, m_data.flashColor.g, m_data.flashColor.b );
      // m_shader.setUniform( "flashColor", targetColor );

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    StrobeData_t m_data;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float intensity;    // From CPU, decays or pulses
uniform float time;         // Total global time (in seconds)
uniform float pulseSpeed;
//uniform vec3 flashColor; // The target color for strobing/fade

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(texture, 0));
    vec4 base = texture2D(texture, uv);

    float flash = intensity;

    // Rhythmic pulsing like a heartbeat (triggered + decaying pulse)
    //flash *= sin(time * pulseSpeed) * exp(-4.0 * intensity); // change 20.0 for pulse speed

    base.rgb = mix(base.rgb, vec3(1.0), flash); // fade-to-white instead of just adding
    //base.rgb += flash;
    //base.rgb = mix(base.rgb, flashColor, flashAmount);
    gl_FragColor = base;
})";
  };
}