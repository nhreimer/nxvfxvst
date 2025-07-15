#pragma once

#include "ParticleLayoutBase.hpp"
#include "models/ParticleBehaviorPipeline.hpp"

namespace nx
{

namespace layout::fractalring
{

#define FRACTAL_RING_LAYOUT_PARAMS(X)                                                                            \
X(depthLimit,               int32_t, 2,      1,    12,   "Max recursive fractal depth",             true)         \
X(baseRingCount,            int32_t, 4,      1,    64,   "Rings in base layer",                     true)         \
X(radiusAdjustment,         float,   0.75f,  0.1f, 5.0f, "Multiplier per level (shrinks or grows)", true)         \
X(radialSpread,             float,   1.0f,   0.0f, 4.0f, "Spread angle per ring",                   true)         \
X(delayFractalFadesMultiplier, float, 1.25f, 0.1f, 5.0f, "Timing multiplier for fade delays",       true)         \
X(enableFractalFades,       bool,    true,   0,    1,    "Fade deeper levels",                      false)        \
X(depthDirection,           int32_t, 1,     -1,    1,    "1 or -1 for ping-pong flow",              false)

  enum class E_FractalDepthTraversalMode
  {
    E_Forward,
    E_Reverse,
    E_PingPong
  };

  struct FractalRingLayoutData_t
  {
    bool isActive = true;
    E_FractalDepthTraversalMode fractalDepthTraversalMode = E_FractalDepthTraversalMode::E_Forward;
    EXPAND_SHADER_PARAMS_FOR_STRUCT(FRACTAL_RING_LAYOUT_PARAMS)
  };

  enum class E_FractalRingParam
  {
    EXPAND_SHADER_PARAMS_FOR_ENUM(FRACTAL_RING_LAYOUT_PARAMS)
    LastItem
  };

  static inline const std::array<std::string, static_cast<size_t>(E_FractalRingParam::LastItem)> m_paramLabels =
  {
    EXPAND_SHADER_PARAM_LABELS(FRACTAL_RING_LAYOUT_PARAMS)
  };
}

///
/// This is good for single notes because it hits the center, like a kick drum or something
class FractalRingLayout final
  : public ParticleLayoutBase< layout::fractalring::FractalRingLayoutData_t >
{
public:

  explicit FractalRingLayout( PipelineContext& context )
    : ParticleLayoutBase( context )
  {
    EXPAND_SHADER_VST_BINDINGS(FRACTAL_RING_LAYOUT_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

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