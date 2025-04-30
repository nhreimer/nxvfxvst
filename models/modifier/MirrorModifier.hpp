#pragma once

#include "../data/PipelineContext.hpp"
#include "models/InterfaceTypes.hpp"

namespace nx
{
  /// this is a class fleshed out for testing purposes only
  class MirrorModifier final : public IParticleModifier
  {

    struct MirrorData_t
    {
      bool isActive = true;
      int count = 4;
      float distance = 50.f;
      float mirrorAlpha = 1.f;
      float lastVelocityNorm = 0.f; // normalized 0.0–1.0
      bool useDynamicRadius = false;
      float dynamicRadius = 1.f;
      float angleOffsetDegrees = 0.f;
      bool useParticleColors = false;

      sf::Color mirrorColor = sf::Color::White;
      sf::Color mirrorOutlineColor = sf::Color::White;
    };

  public:
    explicit MirrorModifier( PipelineContext& context )
      : m_ctx( context )
    {}

    E_ModifierType getType() const override { return E_ModifierType::E_MirrorModifier; }

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    void drawMenu() override;

    void update(const sf::Time &) override {}

    bool isActive() const override { return m_data.isActive; }

    void processMidiEvent(const Midi_t & midi) override
    {
      m_data.lastVelocityNorm = midi.velocity;
    }

    void modify(const ParticleLayoutData_t & layoutData,
                std::deque< TimedParticle_t * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override;

  private:

    PipelineContext& m_ctx;
    MirrorData_t m_data;

  };

} // namespace nx
