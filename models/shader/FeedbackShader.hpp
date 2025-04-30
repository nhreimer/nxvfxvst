#pragma once

#include "models/shader/BlenderShader.hpp"
#include "helpers/CommonHeaders.hpp"

namespace nx
{

  /// not actually a shader. it's a utility to be inserted anywhere in the shader chain.
  class FeedbackShader final : public IShader
  {

#define FEEDBACK_SHADER_PARAMS(X)                                                                   \
X(trailFadeAlpha, int, 8,   0,   255,  "Alpha value subtracted from each trail frame", true)        \
X(fadeColor,      sf::Color, sf::Color::Black, 0, 0,  "Color applied during trail fading", true)    \
X(mixFactor,         float, 1.0f,    0.f,   1.f, "Mix between original and effects result", true)

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
    explicit FeedbackShader( PipelineContext& context );

    ~FeedbackShader() override;

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json& j) override;

    E_ShaderType getType() const override { return E_ShaderType::E_FeedbackShader; }

    void drawMenu() override;

    void update( const sf::Time &deltaTime ) override {}

    void trigger( const Midi_t &midi ) override;

    [[nodiscard]]
    bool isShaderActive() const override;

    [[nodiscard]]
    sf::RenderTexture& applyShader(const sf::RenderTexture& inputTexture) override;

  private:
    PipelineContext& m_ctx;
    FeedbackData_t m_data;

    sf::RectangleShape m_fadeQuad;

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    BlenderShader m_blender;

    TimeEasing m_easing;

  };
}