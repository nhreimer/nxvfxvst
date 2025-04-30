#pragma once

#include "shapes/CurvedLine.hpp"
#include "models/data/ParticleLineData_t.hpp"

namespace nx
{
  /// this works really well with Fractal layouts
  class RingZoneMeshModifier final : public IParticleModifier
  {
    struct RingZoneMeshData_t : public ParticleLineData_t
    {
      bool isActive = true;
      float ringSpacing = 100.f;
      bool drawRings = true;
      bool drawSpokes = true;
      // sf::Color lineColor = sf::Color(255, 255, 255, 100);
      // sf::Color otherLineColor = sf::Color(255, 255, 255, 100);
      //
      // float lineWidth { 2.f };

      float pulseSpeed = 1.0f; // Hz
      float minAlpha = 32.f;
      float maxAlpha = 200.f;
      bool enablePulse = true;
    };

  public:
    explicit RingZoneMeshModifier(PipelineContext& context)
      : m_ctx(context)
    {}

    E_ModifierType getType() const override { return E_ModifierType::E_RingZoneMeshModifier; }

    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;
    void drawMenu() override;
    void update(const sf::Time &) override {}
    bool isActive() const override { return m_data.isActive; }
    void processMidiEvent(const Midi_t &) override {}

    void modify(const ParticleLayoutData_t &, std::deque< TimedParticle_t * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override;

  private:
    static float length(const sf::Vector2f &v) { return std::sqrt(v.x * v.x + v.y * v.y); }

    static float angleFromCenter(const sf::Vector2f &center, const sf::Vector2f &pos)
    {
      const sf::Vector2f d = pos - center;
      return std::atan2(d.y, d.x);
    }

    void setLineColors( CurvedLine * line,
                        const TimedParticle_t * pointA,
                        const TimedParticle_t * pointB,
                        const sf::Color pulsedColor = sf::Color::Transparent ) const;

  private:
    PipelineContext& m_ctx;
    RingZoneMeshData_t m_data;

  };

} // namespace nx
