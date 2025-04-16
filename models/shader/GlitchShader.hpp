#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  struct GlitchData_t
  {
    bool isActive { true };

    float glitchBaseStrength { 0.1f };   // this is adjustable
    float glitchStrength { 1.f };       // [CALCULATED] How intense glitches are (0.0 to 1.0+)
    float glitchAmount { 0.1f };        // How frequent glitches are (0.0 to 1.0)
    float scanlineIntensity { 0.02f };  // Scanline brightness
    float chromaFlickerAmount { 0.4f };  // 0.0 = off, 1.0 = frequent flickers
    // float strobeAmount { 0.0f };         // 0.0 = no strobe, 1.0 = frequent
    float pixelJumpAmount { 0.1f };      // 0.0 = off, 1.0 = chaos

    float glitchPulseDecay { -0.5f };
    float glitchPulseBoost { 1.f };

    float bandCount { 20.f };            // smaller value = bigger chunks
  };

  // deprecated: use the LayeredGlitchShader because it uses layered easings and is smoother
  class GlitchShader final : public IShader
  {

  public:
    explicit GlitchShader( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load glitch fragment shader" );
      }
    }

    ~GlitchShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
   {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "isActive", m_data.isActive },
        { "glitchBaseStrength", m_data.glitchBaseStrength },
        { "glitchAmount", m_data.glitchAmount },
        { "scanlineIntensity", m_data.scanlineIntensity },
        { "chromaFlickerAmount", m_data.chromaFlickerAmount },
        { "pixelJumpAmount", m_data.pixelJumpAmount },
        { "glitchPulseDecay", m_data.glitchPulseDecay },
        { "glitchPulseBoost", m_data.glitchPulseBoost },
        { "bandCount", m_data.bandCount },
        { "easing", m_easing.serialize() },
        { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", false);
        m_data.glitchBaseStrength = j.value("glitchBaseStrength", 1.0f);
        m_data.glitchAmount = j.value("glitchAmount", 0.4f);
        m_data.scanlineIntensity = j.value("scanlineIntensity", 0.02f);
        m_data.chromaFlickerAmount = j.value("chromaFlickerAmount", 0.4f);
        m_data.pixelJumpAmount = j.value("pixelJumpAmount", 0.5f);
        m_data.glitchPulseDecay = j.value("glitchPulseDecay", -0.5f);
        m_data.glitchPulseBoost = j.value("glitchPulseBoost", 2.0f);
        m_data.bandCount = j.value("bandCount", 20.0f);
        m_easing.deserialize( j.at( "easing" ) );
        m_midiNoteControl.deserialize( j.at( "midiTriggers" ) );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_GlitchShader; }

    ///////////////////////////////////////////////////////
    /// IMENUABLE
    ///////////////////////////////////////////////////////

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Glitch" ) )
      {
        ImGui::Checkbox( "Glitch Active##1", &m_data.isActive );

        ImGui::SliderFloat( "Glitch Band Count##1", &m_data.bandCount, 0.f, 50.f );
        ImGui::SliderFloat( "Glitch Base Strength##1", &m_data.glitchBaseStrength, 0.f, 2.0f );
        ImGui::SliderFloat( "Glitch Amount##1", &m_data.glitchAmount, 0.f, 1.0f );
        ImGui::SliderFloat( "Scanline Strength##1", &m_data.scanlineIntensity, 0.f, 1.0f );

        ImGui::SliderFloat( "Chroma Flicker##1", &m_data.chromaFlickerAmount, 0.f, 1.0f );
        ImGui::SliderFloat( "Pixel Jumps##1", &m_data.pixelJumpAmount, 0.f, 1.0f );

        ImGui::Separator();
        ImGui::SliderFloat( "Glitch Burst Boost##1", &m_data.glitchPulseBoost, 0.f, 5.0f );
        ImGui::SliderFloat( "Glitch Burst Decay##1", &m_data.glitchPulseDecay, -10.f, 0.f );
        ImGui::Text( "Glitch Strength %0.2f", m_data.glitchStrength );

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    ///////////////////////////////////////////////////////
    /// ISHADER
    ///////////////////////////////////////////////////////

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture& applyShader( const sf::RenderTexture &inputTexture ) override
    {
      if (m_outputTexture.getSize() != m_winfo.windowSize)
      {
        if (!m_outputTexture.resize(m_winfo.windowSize))
        {
          LOG_ERROR("failed to resize glitch texture");
        }
      }

      const float easedTime = m_easing.getElapsedTime();
      const float easingValue = m_easing.getEasing();

      float boostedStrength = m_data.glitchBaseStrength + easingValue * m_data.glitchPulseBoost;
      m_data.glitchStrength = boostedStrength;

      m_shader.setUniform("easedTime", easedTime);
      m_shader.setUniform("easingValue", easingValue);
      m_shader.setUniform("glitchStrength", boostedStrength);

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));

      m_shader.setUniform("glitchAmount", m_data.glitchAmount);
      m_shader.setUniform("scanlineIntensity", m_data.scanlineIntensity);
      m_shader.setUniform("chromaFlickerAmount", m_data.chromaFlickerAmount);
      m_shader.setUniform("pixelJumpAmount", m_data.pixelJumpAmount);
      m_shader.setUniform("bandCount", m_data.bandCount);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_winfo;

    GlitchData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;

uniform float easedTime;          // seconds since last trigger
uniform float easingValue;        // easing curve [0..1]

uniform float glitchStrength;
uniform float glitchAmount;
uniform float scanlineIntensity;

uniform float chromaFlickerAmount;
uniform float pixelJumpAmount;
uniform float bandCount;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float band = floor(uv.y * bandCount);

    // Drive internal pulse from eased time (local to this burst)
    float pulse = 0.5 + 0.5 * sin(easedTime * 2.0);
    float strength = glitchStrength * pulse * (0.5 + glitchAmount);

    float blockThreshold = 1.0 - glitchAmount * 0.9;
    float jumpThreshold = 1.0 - glitchAmount * 0.8;

    // Block Glitch
    float blockRand = rand(vec2(band, floor(easedTime * 10.0)));
    float blockOffsetX = step(blockThreshold, blockRand) * rand(vec2(band, easedTime)) * 0.2 * strength;
    float blockOffsetY = step(blockThreshold - 0.1, blockRand) * rand(vec2(band + 0.5, easedTime)) * 0.05 * strength;
    uv.x += blockOffsetX;
    uv.y += blockOffsetY;

    // UV Tearing
    float tearIntensity = strength * 0.03 * (0.5 + glitchAmount);
    uv.x += (rand(vec2(easedTime, uv.y * 100.0)) - 0.5) * tearIntensity;
    uv.y += (rand(vec2(uv.x * 100.0, easedTime)) - 0.5) * tearIntensity;

    // Vertical Jump
    float jump = step(jumpThreshold, rand(vec2(floor(easedTime * 2.0), 5.0))) * rand(vec2(easedTime, 0.0)) * 0.1 * strength;
    uv.y += jump;

    // Pixel Jump
    float pixelBlock = 1.0;
    if (step(1.0 - pixelJumpAmount, rand(vec2(floor(easedTime * 4.0), 1.0))) > 0.0) {
        pixelBlock = floor(rand(vec2(easedTime, 2.0)) * 40.0 + 4.0);
        uv = floor(uv * pixelBlock) / pixelBlock;
    }

    // RGB Shift
    vec2 rgbShift = vec2(rand(vec2(easedTime, uv.y)) * 0.005 * strength * (0.5 + glitchAmount), 0.0);

    // Chromatic Flicker
    if (step(1.0 - chromaFlickerAmount, rand(vec2(easedTime * 3.0, 3.0))) > 0.5) {
        rgbShift *= 5.0;
    }

    vec4 r = texture2D(texture, uv + rgbShift);
    vec4 g = texture2D(texture, uv);
    vec4 b = texture2D(texture, uv - rgbShift);

    vec4 color = vec4(r.r, g.g, b.b, 1.0);

    // Scanlines (synced to trigger)
    float scanline = sin(uv.y * resolution.y * 10.0 + easedTime * 40.0) * scanlineIntensity * (0.5 + 0.5 * pulse);
    color.rgb += scanline;

    gl_FragColor = color;
})";

  };

}