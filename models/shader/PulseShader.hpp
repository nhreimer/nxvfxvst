#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  struct PulseData_t
  {
    bool isActive { true };
    float threshold { 0.8f };

    float glowIntensity { 1.2f };
    float burstMultiplier { 1.5f };
    float basePulseThreshold { 0.1f };
  };

  class PulseShader final : public IShader
  {
  public:

    explicit PulseShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_brightPassShader.loadFromMemory( m_brightPassFragmentShader, sf::Shader::Type::Fragment ) ||
           !m_compositeShader.loadFromMemory( m_bloomCompositeFragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load pulse fragment shader" );
      }
    }

    ~PulseShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
   {
      { "type", SerialHelper::serializeEnum( getType() ) },
      { "isActive", m_data.isActive },
      { "threshold", m_data.threshold },
      { "basePulseThreshold", m_data.basePulseThreshold },
      { "burstMultiplier", m_data.burstMultiplier },
         { "easing", m_easing.serialize() },
         { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.threshold = j.value("threshold", 0.8f);
      m_data.basePulseThreshold = j.value("basePulseThreshold", 1.2f);
      m_data.burstMultiplier = j.value("burstMultiplier", 1.5f);
      m_easing.deserialize( j.at( "easing" ) );
      m_midiNoteControl.deserialize( j.at( "midiTriggers" ) );
    }

    E_ShaderType getType() const override { return E_ShaderType::E_PulseShader; }

    void update( const sf::Time &deltaTime ) override {}
    void trigger(const Midi_t &midi) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Pulse" ) )
      {
        ImGui::Checkbox( "Pulse Active##1", &m_data.isActive );

        ImGui::SliderFloat("Pulse Base Threshold##1", &m_data.basePulseThreshold, 0.0f, 5.0f);
        ImGui::SliderFloat("Pulse Burst Mult##1", &m_data.burstMultiplier, 0.0f, 5.0f);

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture) override
    {
      ensureTextureSize();

      const float eased = m_easing.getEasing();
      m_data.glowIntensity = eased * m_data.burstMultiplier;

      // 1. BRIGHT PASS
      m_brightPassShader.setUniform("texture", inputTexture.getTexture());
      // m_brightPassShader.setUniform("threshold", m_data.threshold);

      // make more stuff glow during pulse
      m_brightPassShader.setUniform("threshold", m_data.basePulseThreshold - eased * 0.2f);

      m_brightTexture.clear();
      m_brightTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_brightPassShader);
      m_brightTexture.display();

      // 2. COMPOSITE PASS
      m_compositeShader.setUniform("baseTexture", inputTexture.getTexture());
      m_compositeShader.setUniform("bloomTexture", m_brightTexture.getTexture());
      m_compositeShader.setUniform("intensity", m_data.glowIntensity);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_compositeShader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    void ensureTextureSize()
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) ||
             !m_brightTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize pulse texture" );
        }
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    PulseData_t m_data;

    sf::Clock m_clock;

    sf::Shader m_brightPassShader;
    sf::Shader m_compositeShader;

    sf::RenderTexture m_brightTexture;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_brightPassFragmentShader = R"(uniform sampler2D texture;
uniform float threshold; // e.g. 0.8

void main() {
    vec4 color = texture2D(texture, gl_FragCoord.xy / vec2(textureSize(texture, 0)));
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); // Luminance
    gl_FragColor = brightness > threshold ? color : vec4(0.0);
})";

    const static inline std::string m_bloomCompositeFragmentShader = R"(uniform sampler2D baseTexture;   // original image
uniform sampler2D bloomTexture;  // blurred glow

uniform float intensity;         // blend strength

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(baseTexture, 0));
    vec4 base = texture2D(baseTexture, uv);
    vec4 glow = texture2D(bloomTexture, uv);
    gl_FragColor = base + glow * intensity;
})";
  };

}