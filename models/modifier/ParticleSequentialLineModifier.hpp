#pragma once

#include "shapes/CurvedLine.hpp"

#include "models/data/ParticleLineData_t.hpp"

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  class ParticleSequentialLineModifier final : public IParticleModifier
  {
  public:
    explicit ParticleSequentialLineModifier( PipelineContext& context )
      : m_ctx( context )
    {}

    void drawMenu() override;

    bool isActive() const override { return m_isActive; }

    void processMidiEvent(const Midi_t &midiEvent) override {}

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    E_ModifierType getType() const override { return E_ModifierType::E_SequentialModifier; }

    void update( const sf::Time &deltaTime ) override {}

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override;

  private:

    void setLineColors( CurvedLine * line,
                        const TimedParticle_t * pointA,
                        const TimedParticle_t * pointB ) const;

  private:

    PipelineContext& m_ctx;
    bool m_isActive { true };
    ParticleLineData_t m_data;

  };

}