#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <shapes/MidiNoteControl.hpp>

namespace nx
{

  struct BlurData_t
  {
    bool isActive { true };

    float sigma { 7.f };
    float brighten { 1.f };
    float blurHorizontal { 0.1f };
    float blurVertical { 0.1f };

    // used for triggers
    float impactIntensity { 0.f };  // [calculated for us]
    float impactTimeDecay { 1.5f }; // in seconds
  };

  class BlurShader final : public IShader
  {
  public:

    explicit BlurShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_blurShader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
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
          { "type", SerialHelper::convertShaderTypeToString( getType() ) },
          { "isActive", m_data.isActive },
          { "sigma", m_data.sigma },
          { "brighten", m_data.brighten },
          { "blurHorizontal", m_data.blurHorizontal },
          { "blurVertical", m_data.blurVertical },
          { "impactTimeDecay", m_data.impactTimeDecay },
          { "midiTriggers", m_midiNoteControl.serialize() }
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.sigma = j.value("sigma", 7.f);
      m_data.brighten = j.value("brighten", 1.f);
      m_data.blurHorizontal = j.value("blurHorizontal", 0.f);
      m_data.blurVertical = j.value("blurVertical", 0.f);
      m_data.impactTimeDecay = j.value("impactTimeDecay", 0.5f);
      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
    }

    // identify type for easier loading
    E_ShaderType getType() const override { return E_BlurShader; }

    void update( const sf::Time& deltaTime ) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Image Blur" ) )
      {
        ImGui::Checkbox( "Blur Active##1", &m_data.isActive );
        ImGui::SliderFloat( "Smoothing##1", &m_data.sigma, 0.f, 50.f );
        ImGui::SliderFloat( "Brighten##1", &m_data.brighten, 1.f, 5.f );
        ImGui::SliderFloat( "Horizontal##1", &m_data.blurHorizontal, 0.f, 5.f );
        ImGui::SliderFloat( "Vertical##1", &m_data.blurVertical, 0.f, 5.f );

        ImGui::Separator();
        ImGui::SliderFloat("Impact Decay##1", &m_data.impactTimeDecay, 0.f, 1.f);
        ImGui::Text("Impact Intensity %0.2f##1", &m_data.impactIntensity);

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void trigger( const Midi_t& midi ) override
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      {
        // Set intensity to max and restart the clock
        m_data.impactIntensity = 1.0f;
        m_clock.restart();
      }
    }

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.blurHorizontal + m_data.blurVertical > 0.f; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) ||
             !m_intermediary.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize blur texture" );
        }
      }

      // Compute time since last trigger
      const float elapsed = m_clock.getElapsedTime().asSeconds();

      // Decay intensity over time
      m_data.impactIntensity = std::max( 0.f, 1.0f - ( elapsed / m_data.impactTimeDecay ) );

      const sf::Sprite sprite( inputTexture.getTexture() );

      // Apply horizontal blur
      m_blurShader.setUniform( "texture", inputTexture.getTexture() );
      m_blurShader.setUniform( "direction", sf::Glsl::Vec2( 1.f, 0.f ) ); // Horizontal
      m_blurShader.setUniform( "blurRadiusX", m_data.blurHorizontal );
      m_blurShader.setUniform( "blurRadiusY", 0.f ); // No vertical blur in this pass
      m_blurShader.setUniform( "sigma", m_data.sigma );
      m_blurShader.setUniform( "brighten", m_data.brighten );
      m_blurShader.setUniform( "intensity", m_data.impactIntensity );

      m_intermediary.clear(sf::Color::Transparent);
      m_intermediary.draw(sprite, &m_blurShader);
      m_intermediary.display();

      // Apply vertical blur
      m_blurShader.setUniform("texture", m_intermediary.getTexture());
      m_blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f)); // Vertical
      m_blurShader.setUniform("blurRadiusX", 0.f); // No horizontal blur in this pass
      m_blurShader.setUniform("blurRadiusY", m_data.blurVertical);
      m_blurShader.setUniform( "sigma", m_data.sigma );
      m_blurShader.setUniform( "brighten", m_data.brighten );
      m_blurShader.setUniform( "intensity", m_data.impactIntensity );

      m_outputTexture.clear(sf::Color::Transparent);
      m_outputTexture.draw(sprite, &m_blurShader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    sf::Shader m_blurShader;
    sf::RenderTexture m_intermediary;
    sf::RenderTexture m_outputTexture;

    BlurData_t m_data;

    sf::Clock m_clock;
    MidiNoteControl m_midiNoteControl;
    float m_lastTriggerTime { INT32_MIN };

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float blurRadiusX;
uniform float blurRadiusY;
uniform vec2 direction;
uniform float sigma;
uniform float brighten;
uniform float intensity; // Amplifies both blur and brightness
uniform float decay;

// Gaussian function
float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.50662827463 * sigma);
}

void main() {
    vec2 texSize = textureSize(texture, 0);

    // Apply intensity to sigma (blur)
    float amplifiedSigma = sigma * (1.0 + intensity);
    vec2 texOffset = direction * vec2(blurRadiusX / texSize.x, blurRadiusY / texSize.y);

    // Center sample
    vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gaussian(0.0, amplifiedSigma);
    float totalWeight = gaussian(0.0, amplifiedSigma);

    // Blur samples
    for (int i = 1; i <= 10; i++) {
        float offset = float(i);
        float weight = gaussian(offset, amplifiedSigma);

        color += texture2D(texture, gl_TexCoord[0].xy + texOffset * offset) * weight;
        color += texture2D(texture, gl_TexCoord[0].xy - texOffset * offset) * weight;
        totalWeight += 2.0 * weight;
    }

    // Apply brighten + intensity boost
    float amplifiedBrighten = brighten + intensity;

    gl_FragColor = ( color * amplifiedBrighten ) / totalWeight;
})";

  };
}