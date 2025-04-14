#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  struct RumbleData_t
  {
    bool isActive { true };

    float rumbleStrength { 20.f };      // max px offset
    float frequency { 40.f };           // wiggle speed
    float pulseDecay { -5.f };          // how fast the shake fades
    sf::Vector2f direction { 1.f, 1.f }; // shake X, Y
    bool useNoise { false };            // toggle for noise mode

    float modAmplitude { 0.5f };   //  0.0–1.0 wobble amount
    float modFrequency { 10.f };   //  0.0–50.0 wobble speed
    float colorDesync { 1.0f };    //  RGB shift multiplier

    // adjust how much the colors come detach from the center of the particle
    float baseColorDesync { 0.25f };
    float maxColorDesync { 0.25f };
  };

  class RumbleShader final : public IShader
  {
  public:

    explicit RumbleShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load rumble fragment shader" );
      }
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
   {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "isActive", m_data.isActive },
        { "rumbleStrength", m_data.rumbleStrength },
        { "frequency", m_data.frequency },
        { "pulseDecay", m_data.pulseDecay },
        { "direction", { m_data.direction.x, m_data.direction.y } },
          { "useNoise", m_data.useNoise },
          { "modAmplitude", m_data.modAmplitude },
          { "modFrequency", m_data.modFrequency },
          { "baseColorDesync", m_data.baseColorDesync },
            { "maxColorDesync", m_data.maxColorDesync },
        { "easing", m_easing.serialize() },
          { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }
    void deserialize( const nlohmann::json &j ) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.rumbleStrength = j.value("rumbleStrength", 20.f);
      m_data.frequency = j.value("frequency", 40.f);
      m_data.pulseDecay = j.value("pulseDecay", -5.f);
      m_data.direction = SerialHelper::convertVectorFromJson< float >( j.at( "direction" ), sf::Vector2f{ 1.f, 1.f } );
      m_data.useNoise = j.value("useNoise", false);
      m_data.modAmplitude = j.value("modAmplitude", 0.5f);
      m_data.modFrequency = j.value("modFrequency", 10.f);
      m_data.baseColorDesync = j.value("baseColorDesync", 0.25f);
      m_data.maxColorDesync = j.value("maxColorDesync", 0.25f);
      m_easing.deserialize( j.at( "easing" ) );
      m_midiNoteControl.deserialize( j.at( "midiTriggers" ) );
    }

    [[nodiscard]]
    E_ShaderType getType() const override { return E_RumbleShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode("Rumble" ) )
      {
        ImGui::Checkbox("Active", &m_data.isActive);
        ImGui::Checkbox("Use Noise Shake", &m_data.useNoise);

        ImGui::Separator();
        ImGui::SliderFloat("Strength", &m_data.rumbleStrength, 0.f, 100.f);
        ImGui::SliderFloat("Frequency", &m_data.frequency, 0.f, 100.f);
        ImGui::SliderFloat("Pulse Decay", &m_data.pulseDecay, -20.f, -0.1f);
        ImGui::SliderFloat("Direction X", &m_data.direction.x, 0.f, 1.f);
        ImGui::SliderFloat("Direction Y", &m_data.direction.y, 0.f, 1.f);

        ImGui::Separator();
        ImGui::SliderFloat("Modulation Amplitude", &m_data.modAmplitude, 0.f, 2.0f);
        ImGui::SliderFloat("Modulation Frequency", &m_data.modFrequency, 0.f, 50.f);
        ImGui::SliderFloat("Color Desync", &m_data.colorDesync, 0.f, 5.0f);

        ImGui::SliderFloat("Base Color Desync", &m_data.baseColorDesync, 0.f, 2.0f);
        ImGui::SliderFloat("Max Color Desync", &m_data.maxColorDesync, 0.f, 1.5f );

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
    sf::RenderTexture &applyShader(const sf::RenderTexture &inputTexture) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize rumble texture" );
        }
      }

      const float time = m_clock.getElapsedTime().asSeconds();
      const float pulse = m_easing.getEasing();

      m_shader.setUniform("texture", inputTexture.getTexture());
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture.getSize()));
      m_shader.setUniform("time", time);

      m_shader.setUniform("rumbleStrength", m_data.rumbleStrength);
      m_shader.setUniform("frequency", m_data.frequency);
      m_shader.setUniform("pulseValue", pulse); // now driven by easing
      m_shader.setUniform("direction", sf::Glsl::Vec2(m_data.direction));
      m_shader.setUniform("useNoise", m_data.useNoise);

      m_shader.setUniform("modAmplitude", m_data.modAmplitude);
      m_shader.setUniform("modFrequency", m_data.modFrequency);
      m_shader.setUniform("colorDesync", pulse * m_data.maxColorDesync + m_data.baseColorDesync);
      //m_shader.setUniform("colorDesync", m_data.colorDesync);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    RumbleData_t m_data;

    sf::Clock m_clock;
    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;
    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;

uniform float rumbleStrength;
uniform float frequency;
uniform float pulseValue;
uniform vec2 direction;
uniform bool useNoise;

// Amplitude modulation
uniform float modAmplitude;
uniform float modFrequency;

// RGB offset
uniform float colorDesync;

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    float ampMod = 1.0 + sin(time * modFrequency) * modAmplitude;

    float shakeX = 0.0;
    float shakeY = 0.0;

    if (useNoise) {
        shakeX = (hash(time * frequency) - 0.5) * 2.0 * direction.x;
        shakeY = (hash(time * frequency + 100.0) - 0.5) * 2.0 * direction.y;
    } else {
        shakeX = sin(time * frequency) * direction.x;
        shakeY = cos(time * frequency) * direction.y;
    }

    vec2 shake = vec2(shakeX, shakeY) * (rumbleStrength * ampMod / resolution) * pulseValue;

    // Color desync (red and blue shift opposite directions)
    vec2 redUV = uv + shake * colorDesync;
    vec2 greenUV = uv;
    vec2 blueUV = uv - shake * colorDesync;

    float r = texture2D(texture, redUV).r;
    float g = texture2D(texture, greenUV).g;
    float b = texture2D(texture, blueUV).b;

    gl_FragColor = vec4(r, g, b, 1.0);
})";

  };
}