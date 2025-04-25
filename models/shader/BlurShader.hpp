#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class BlurShader final : public IShader
  {

#define BLUR_SHADER_PARAMS(X)                        \
X(sigma,         float, 7.f,     0.f,   50.f , "Amount of blurring")   \
X(brighten,      float, 1.f,     0.f,   5.f  , "Brightens the blurred areas")   \
X(blurHorizontal,float, 1.0f,    0.f,   20.f , "Blurs in the horizontal direction")   \
X(blurVertical,  float, 1.0f,    0.f,   20.f , "Blurs in the vertical direction")

    struct BlurData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(BLUR_SHADER_PARAMS)
    };

    enum class E_BlurParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(BLUR_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_BlurParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(BLUR_SHADER_PARAMS)
    };

  public:

    explicit BlurShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      if ( !m_blurShader.loadFromMemory(m_fragmentShader, sf::Shader::Type::Fragment) )
      {
        LOG_ERROR("Failed to load blur fragment shader");
      }
      else
      {
        LOG_INFO("loaded blur shader");
      }
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(BLUR_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize( const nlohmann::json& j ) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(BLUR_SHADER_PARAMS)

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
      if ( ImGui::TreeNode( "Gaussian Blur Options" ) )
      {
        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        auto& STRUCT_REF = m_data;
        BLUR_SHADER_PARAMS(X_SHADER_IMGUI);

        ImGui::SeparatorText( "Easings" );
        m_easing.drawMenu();

        ImGui::SeparatorText( "Midi Triggers" );
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

      const float easing = m_easing.getEasing();

      const sf::Sprite sprite( inputTexture.getTexture() );

      // Apply horizontal blur
      m_blurShader.setUniform( "texture", inputTexture.getTexture() );
      m_blurShader.setUniform( "direction", sf::Glsl::Vec2( 1.f, 0.f ) ); // Horizontal
      m_blurShader.setUniform( "blurRadiusX", m_data.blurHorizontal );
      m_blurShader.setUniform( "blurRadiusY", 0.f ); // No vertical blur in this pass
      m_blurShader.setUniform( "sigma", m_data.sigma );
      m_blurShader.setUniform( "brighten", m_data.brighten );

      m_blurShader.setUniform( "intensity", easing );

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
      m_blurShader.setUniform( "intensity",easing );

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

    MidiNoteControl m_midiNoteControl;
    TimeEasing m_easing;

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