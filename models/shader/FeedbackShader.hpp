#pragma once

#include "helpers/CommonHeaders.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  /// not actually a shader. it's a utility to be inserted anywhere in the shader chain.
  class FeedbackShader final : public IShader
  {

#define FEEDBACK_SHADER_PARAMS(X)                                                             \
X(trailFadeAlpha, int, 8,   0,   255,  "Alpha value subtracted from each trail frame")        \
X(fadeColor,      sf::Color, sf::Color::Black, 0, 0,  "Color applied during trail fading")    \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result")

    struct FeedbackData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(FEEDBACK_SHADER_PARAMS)
    };

    enum class E_FeedbackParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(FEEDBACK_SHADER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FeedbackParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(FEEDBACK_SHADER_PARAMS)
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
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum(getType());
      EXPAND_SHADER_PARAMS_TO_JSON(FEEDBACK_SHADER_PARAMS)
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void deserialize(const nlohmann::json& j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(FEEDBACK_SHADER_PARAMS)
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
        m_easing.setEasingType( E_TimeEasingType::E_Disabled );
      }
    }

    E_ShaderType getType() const override { return E_ShaderType::E_FeedbackShader; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Feedback Options" ) )
      {
        auto& STRUCT_REF = m_data;
        FEEDBACK_SHADER_PARAMS(X_SHADER_IMGUI);

        if ( ImGui::SmallButton( "Clear" ) )
          m_outputTexture.clear( sf::Color::Transparent );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override
    {
      m_easing.trigger();
    }

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

      const auto clampedEasing = std::clamp( m_easing.getEasing(), 0.f, 1.f );

      // Resize fade quad
      m_fadeQuad.setSize(targetSize);
      m_fadeQuad.setFillColor(sf::Color(m_data.fadeColor.r,
                                            m_data.fadeColor.g,
                                            m_data.fadeColor.b,
                                            static_cast<uint8_t>(m_data.trailFadeAlpha * clampedEasing)));

      // Step 1: Fade out previous trail
      m_outputTexture.draw(m_fadeQuad, sf::BlendAlpha);

      // Step 2: Draw new input frame on top
      m_outputTexture.draw(sf::Sprite(inputTexture.getTexture()), sf::BlendAdd);

      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                                    m_outputTexture,
                                    m_data.mixFactor );
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    FeedbackData_t m_data;

    sf::RectangleShape m_fadeQuad;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;

    TimeEasing m_easing;

  };
}