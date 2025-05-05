#pragma once

#include "models/ParticleBehaviorPipeline.hpp"

namespace nx
{

struct FractalRingLayoutData_t : public ParticleLayoutData_t
{
  int32_t depthLimit { 2 };
  int32_t baseRingCount { 4 };
  float radiusAdjustment { 0.75f };
  float radialSpread { 1.f };
  float delayFractalFadesMultiplier { 1.25f };
  bool enableFractalFades { true };
};

///
/// This is good for single notes because it hits the center, like a kick drum or something
class FractalRingLayout final : public IParticleLayout
{
public:

  explicit FractalRingLayout( PipelineContext& context )
    : m_ctx( context ),
      m_behaviorPipeline( context )
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override;

  void deserialize(const nlohmann::json &j) override;

  [[nodiscard]]
  E_LayoutType getType() const override { return E_LayoutType::E_FractalRingLayout; }

  void addMidiEvent( const Midi_t &midiEvent ) override;

  void update( const sf::Time &deltaTime ) override;

  void drawMenu() override;

  [[nodiscard]]
  const ParticleLayoutData_t & getParticleOptions() const override { return m_data; }

  [[nodiscard]]
  std::deque< TimedParticle_t * > & getParticles() override { return m_particles; }

private:

  void spawnFractalRing( const Midi_t& midiEvent,
                         const int depth,
                         const float adjustedRadius,
                         const sf::Vector2f& lastPosition );

  TimedParticle_t* createParticle( const Midi_t& midiEvent,
                                   const sf::Vector2f& position,
                                   const float adjustedRadius );

private:
  PipelineContext& m_ctx;
  std::deque< TimedParticle_t * > m_particles;
  FractalRingLayoutData_t m_data;

  ParticleBehaviorPipeline m_behaviorPipeline;
  int32_t m_currentDepth { 1 };
};

}