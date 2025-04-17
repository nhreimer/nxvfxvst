#pragma once

#include "helpers/CommonHeaders.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  /// not actually a shader. it's a utility to be inserted anywhere in the shader chain.
  class FeedbackShader final : public IShader
  {

    struct FeedbackData_t
    {
      bool isActive { true };

      // 0    -> trail never fades (fully persistent)
      int32_t trailFadeAlpha { 8 };
    };

  public:
    explicit FeedbackShader( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    ~FeedbackShader() override = default;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
          { "type", SerialHelper::serializeEnum( getType() ) },
          { "isActive", m_data.isActive },
           { "trailFadeAlpha", m_data.trailFadeAlpha }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j.value("isActive", false);
        m_data.trailFadeAlpha = j.value("trailFadeAlpha", 0);
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_FeedbackShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Feedback" ) )
      {
        ImGui::Checkbox( "Feedback Active##1", &m_data.isActive );
        ImGui::SliderInt("Fade Factor (Feedback)", &m_data.trailFadeAlpha, 0, 255);

        if ( ImGui::SmallButton( "Clear" ) )
          m_outputTexture.clear( sf::Color::Transparent );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {}

    [[nodiscard]]
    bool isShaderActive() const override { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture) override
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR("Failed to resize feedback textures");
        }
      }

      const auto targetSize = sf::Vector2f{ m_globalInfo.windowSize };

      // Resize fade quad
      m_fadeQuad.setSize(targetSize);
      m_fadeQuad.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(m_data.trailFadeAlpha)));

      // Step 1: Fade out previous trail
      m_outputTexture.draw(m_fadeQuad, sf::BlendAlpha);

      // Step 2: Draw new input frame on top
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), sf::BlendAdd);

      m_outputTexture.display();
      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    FeedbackData_t m_data;

    sf::RectangleShape m_fadeQuad;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

  };
}