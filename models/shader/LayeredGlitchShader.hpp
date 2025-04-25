#pragma once

#include "helpers/CommonHeaders.hpp"


#include "models/shader/easings/CumulativeEasing.hpp"
#include "shapes/BPMSelector.hpp"

namespace nx
{

  class LayeredGlitchShader final : public IShader
  {

// easings:    glitchBaseStrength, glitchAmount, and pixelJumpAmount

#define GLITCH_SHADER_PARAMS(X)                                                               \
X(applyOnlyOnEvents, bool,  false, 0, 0,  "Pause rendering between glitches until retrigger") \
X(glitchBaseStrength, float, 0.1f, 0.f, 5.f, "Base glitch strength when idle")                \
X(glitchStrength, float,     1.f,   0.f, 5.f, "[CALC] Final glitch strength from pulse")      \
X(glitchAmount, float,       0.1f,  0.f, 1.f, "Glitch frequency (0 = off, 1 = constant)")     \
X(chromaFlickerAmount, float, 0.4f, 0.f, 1.f, "Chance of heavy RGB flicker per frame")        \
X(pixelJumpAmount, float,    0.1f,  0.f, 1.f, "Chance of randomized blocky pixel jumps")      \
X(glitchPulseDecay, float,  -0.5f, -10.f, 0.0f, "Rate at which glitch bursts decay")          \
X(glitchPulseBoost, float,   1.f,   0.f, 5.f, "How much glitch intensity is added per burst") \
X(bandCount, float,         20.f,   2.f, 60.f, "Blockiness of glitch scanlines (low = big)")

    struct LayeredGlitchData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(GLITCH_SHADER_PARAMS)
    };

    enum class E_GlitchParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(GLITCH_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_GlitchParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(GLITCH_SHADER_PARAMS)
    };

  public:
    explicit LayeredGlitchShader( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load glitch fragment shader" );
      }
    }

    ~LayeredGlitchShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(GLITCH_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_burstManager.serialize();
      return j;
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(GLITCH_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_burstManager.deserialize( j[ "easing" ] );
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
      if ( ImGui::TreeNode( "Glitch Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        GLITCH_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::Separator();
        m_burstManager.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    ///////////////////////////////////////////////////////
    /// ISHADER
    ///////////////////////////////////////////////////////

    void update( const sf::Time &deltaTime ) override
    {
      m_burstManager.update( deltaTime.asSeconds() );
    }

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      {
        m_burstManager.trigger();
        m_clock.restart();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.glitchStrength > 0.f; }

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

      const float cumulative = m_burstManager.getEasing();
      const float boostedStrength = m_data.glitchBaseStrength + cumulative * m_data.glitchPulseBoost;

      m_shader.setUniform("glitchStrength", boostedStrength);
      //m_shader.setUniform("easingValue", cumulative); // optional, for shader-side sync

      // determine whether to apply cumulative triggers only, which provides a staccato feel, especially
      // on fast beats, but it can be weird on slower events
      if ( !m_data.applyOnlyOnEvents )
        m_shader.setUniform("easedTime", m_clock.getElapsedTime().asSeconds() );
      else
        m_shader.setUniform("easedTime", m_burstManager.getLastTriggeredInSeconds() );

      m_shader.setUniform("glitchStrength", boostedStrength);

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));

      m_shader.setUniform("glitchAmount", m_data.glitchAmount);
      // m_shader.setUniform("scanlineIntensity", m_data.scanlineIntensity);
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

    LayeredGlitchData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    MidiNoteControl m_midiNoteControl;

    CumulativeEasing m_burstManager;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;

uniform float easedTime;          // seconds since last trigger
//uniform float easingValue;           // easing curve [0..1] -- cumulative easing

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

    gl_FragColor = color;
})";

  };

}