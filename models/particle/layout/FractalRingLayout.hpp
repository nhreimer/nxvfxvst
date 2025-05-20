#pragma once

#include "ParticleLayoutBase.hpp"
#include "models/ParticleBehaviorPipeline.hpp"

namespace nx
{

  enum class E_FractalDepthTraversalMode
  {
    E_Forward,
    E_Reverse,
    E_PingPong
  };

struct FractalRingLayoutData_t : public ParticleLayoutData_t
{
  int32_t depthLimit { 2 };
  int32_t baseRingCount { 4 };
  float radiusAdjustment { 0.75f };
  float radialSpread { 1.f };
  float delayFractalFadesMultiplier { 1.25f };
  bool enableFractalFades { true };
  E_FractalDepthTraversalMode fractalDepthTraversalMode { E_FractalDepthTraversalMode::E_Forward };
  int32_t depthDirection { 1 }; // +1 or -1 for ping-pong
};

///
/// This is good for single notes because it hits the center, like a kick drum or something
class FractalRingLayout final : public ParticleLayoutBase< FractalRingLayoutData_t >
{
public:

  explicit FractalRingLayout( PipelineContext& context )
    : ParticleLayoutBase( context )
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override;

  void deserialize(const nlohmann::json &j) override;

  [[nodiscard]]
  E_LayoutType getType() const override { return E_LayoutType::E_FractalRingLayout; }

  void addMidiEvent( const Midi_t &midiEvent ) override;

  void drawMenu() override;

private:

  void spawnFractalRing( const Midi_t& midiEvent,
                         const int depth,
                         const float adjustedRadius,
                         const sf::Vector2f& lastPosition );

  IParticle * createParticle( const Midi_t& midiEvent,
                              const float adjustedRadius );

private:

  int32_t m_currentDepth { 1 };

};

}