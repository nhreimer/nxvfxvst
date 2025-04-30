#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{
  class ColorMorphBehavior final : public IParticleBehavior
  {

    struct ColorMorphData_t
    {
      float speed = 1.0f;         // cycles per second
      float saturation = 1.0f;    // color intensity
      float brightness = 1.0f;    // brightness
      float quantizeStep = 0.f;   // set > 0 to enable hue steps
      float hueOffset = 0.f;
      float morphDuration = 2.f;
      bool useHueOffset = true;
      bool useSkittles = false;   // just a color free-for-all
      bool reverseColors = false;
      sf::Color morphToColor { sf::Color::Black };
    };

  public:
    explicit ColorMorphBehavior(PipelineContext& context)
      : m_ctx(context)
    {}

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