#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class ColorMorphBehavior final : public IParticleBehavior
  {

#define COLOR_MORPH_BEHAVIOR_PARAMS(X)                                                                   \
X(speed,         float,     1.0f,   0.0f, 10.0f,   "Hue cycle speed (Hz)",            true)             \
X(saturation,    float,     1.0f,   0.0f, 1.0f,    "Color saturation multiplier",     true)             \
X(brightness,    float,     1.0f,   0.0f, 1.0f,    "Brightness multiplier",           true)             \
X(quantizeStep,  float,     0.0f,   0.0f, 90.0f,   "Set > 0 to enable hue steps",     true)             \
X(hueOffset,     float,     0.0f,  -1.0f, 1.0f,    "Hue offset (normalized)",         true)             \
X(morphDuration, float,     2.0f,   0.01f, 10.0f,  "Duration of each morph (sec)",    true)             \
X(useHueOffset,  bool,      true,   0,     1,      "Toggle hue offset",               true)            \
X(useSkittles,   bool,      false,  0,     1,      "Enable Skittles color mode",      true)            \
X(reverseColors, bool,      false,  0,     1,      "Reverse the hue morph direction", true)            \
X(morphToColor,  sf::Color, sf::Color::Black, 0, 255, "Target color for hue morph",   false)

    struct ColorMorphData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(COLOR_MORPH_BEHAVIOR_PARAMS)
    };

    enum class E_ColorMorphParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(COLOR_MORPH_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_ColorMorphParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(COLOR_MORPH_BEHAVIOR_PARAMS)
    };

  public:
    explicit ColorMorphBehavior(PipelineContext& context)
      : m_ctx(context)
    {
      EXPAND_SHADER_VST_BINDINGS(COLOR_MORPH_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_ColorMorphBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override;
    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override;

    void drawMenu() override;

  private:

    void applyBicolorMorphing( TimedParticle_t * p, const sf::Time& deltaTime ) const;
    void applyBicolorMorphingReverse( TimedParticle_t * p, const sf::Time& deltaTime ) const;
    void applySkittlesMorphing( TimedParticle_t * p, const sf::Time& deltaTime ) const;

    static sf::Color hsvToRgb(float h, float s, float v, uint8_t alpha = 255);

  private:
    PipelineContext& m_ctx;
    ColorMorphData_t m_data;
  };
}