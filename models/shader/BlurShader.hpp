#pragma once

#include <SFML/Graphics/Shader.hpp>

namespace nx
{

  struct BlurData_t
  {
    bool isActive { false };

    float sigma { 7.f };
    float brighten { 1.f };
    float blurHorizontal { 0.f };
    float blurVertical { 0.f };
  };

  class BlurShader final : public IShader
  {
  public:

    explicit BlurShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {
      assert( m_blurShader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) );
      LOG_INFO( "loaded blur shader" );
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", getType() },
          { "isActive", m_data.isActive },
          { "sigma", m_data.sigma },
          { "brighten", m_data.brighten },
          { "blurHorizontal", m_data.blurHorizontal },
          { "blurVertical", m_data.blurVertical }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.isActive = j.value("isActive", false);
      m_data.sigma = j.value("sigma", 7.f);
      m_data.brighten = j.value("brighten", 1.f);
      m_data.blurHorizontal = j.value("blurHorizontal", 0.f);
      m_data.blurVertical = j.value("blurVertical", 0.f);
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

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void trigger( const Midi_t& midi ) override {}

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive && m_data.blurHorizontal + m_data.blurVertical > 0.f; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) || !m_intermediary.resize( m_globalInfo.windowSize ) )
          LOG_ERROR( "failed to resize blur texture" );
        else
          LOG_INFO( "successfully resized blur texture" );
      }

      const sf::Sprite sprite( inputTexture.getTexture() );

      // Apply horizontal blur
      m_blurShader.setUniform( "texture", inputTexture.getTexture() );
      m_blurShader.setUniform( "direction", sf::Glsl::Vec2( 1.f, 0.f ) ); // Horizontal
      m_blurShader.setUniform( "blurRadiusX", m_data.blurHorizontal );
      m_blurShader.setUniform( "blurRadiusY", 0.f ); // No vertical blur in this pass
      m_blurShader.setUniform( "sigma", m_data.sigma );
      m_blurShader.setUniform( "brighten", m_data.brighten );

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

    const static inline std::string m_fragmentShader = R"(uniform sampler2D texture;
uniform float blurRadiusX; // Controls horizontal blur
uniform float blurRadiusY; // Controls vertical blur
uniform vec2 direction;    // (1,0) for horizontal, (0,1) for vertical
uniform float sigma;
uniform float brighten;

// Gaussian function approximation for weights
float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.50662827463 * sigma); // 1/sqrt(2πσ²)
}

void main() {
    vec2 texSize = textureSize(texture, 0);
    vec2 texOffset = direction * vec2(blurRadiusX / texSize.x, blurRadiusY / texSize.y);

    //float sigma = 7.0;  // Standard deviation: Higher = smoother
    vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gaussian(0.0, sigma);
    float totalWeight = gaussian(0.0, sigma);

    // Use 10 samples on each side (21 total samples)
    for (int i = 1; i <= 10; i++) {
        float weight = gaussian(float(i), sigma);
        color += texture2D(texture, gl_TexCoord[0].xy + texOffset * float(i)) * weight;
        color += texture2D(texture, gl_TexCoord[0].xy - texOffset * float(i)) * weight;
        totalWeight += 2.0 * weight;  // Account for both sides
    }

    gl_FragColor = ( color * brighten ) / totalWeight;  // Normalize to prevent brightening
})";

  };
}