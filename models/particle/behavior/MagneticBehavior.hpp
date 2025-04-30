#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{

  class MagneticAttractorBehavior final : public IParticleBehavior
  {

    struct MagneticData_t
    {
      bool isAttracting = true;
      float strength = 100.f;
      bool useFalloff = true;
      float falloffExponent = 1.0f;
      //bool followMouse = false;
      sf::Vector2f magnetLocation { 0.5f, 0.5f };
    };

  public:
    explicit MagneticAttractorBehavior(PipelineContext& context)
      : m_ctx( context )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }
    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_BehaviorType getType() const override { return E_BehaviorType::E_MagneticBehavior; }

    void applyOnSpawn(TimedParticle_t *, const Midi_t&) override
    {}

    void applyOnUpdate(TimedParticle_t * p, const sf::Time& dt) override;

    void drawMenu() override;

  private:

    static float length(const sf::Vector2f& v)
    {
      return std::sqrt(v.x * v.x + v.y * v.y);
    }

  private:
    PipelineContext m_ctx;

    MagneticData_t m_data;
    TimedCursorPosition m_timedCursor;
  };


}