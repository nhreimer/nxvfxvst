#pragma once

#include "helpers/CommonHeaders.hpp"
#include "models/data/ParticleLineData_t.hpp"
#include "shapes/CurvedLine.hpp"
#include "helpers/ColorHelper.hpp"

namespace nx
{

  class ParticleFullMeshLineModifier final : public IParticleModifier
  {
  public:

    explicit ParticleFullMeshLineModifier( PipelineContext& context )
      : m_ctx( context )
    {}

    nlohmann::json serialize() const override;

    void deserialize( const nlohmann::json& j ) override;

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    E_ModifierType getType() const override { return E_ModifierType::E_FullMeshModifier; }

    void drawMenu() override;

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