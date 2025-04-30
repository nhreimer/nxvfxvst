#pragma once

#include "models/particle/ParticleLayoutBase.hpp"

namespace nx
{

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class LSystemCurveLayout final : public IParticleLayout
  {

    enum class E_BranchMode : int8_t
    {
      E_Both,
      E_LeftOnly,
      E_RightOnly,
      E_MidiPitch
    };

    struct LSystemCurveLayoutData_t : public ParticleLayoutData_t
    {
      int depth = 4; // number of recursive steps
      float turnAngle = 25.f; // degrees turned on L/R
      float segmentLength = 20.f; // length of each forward step
      float initialAngleDeg = 0.f;
      E_BranchMode m_branchMode = E_BranchMode::E_Both;
    };

  public:
    explicit LSystemCurveLayout(PipelineContext& context)
      : m_ctx( context ),
        m_behaviorPipeline( context )
    {}

    ~LSystemCurveLayout() override;

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_LayoutType getType() const override { return E_LayoutType::E_LSystemCurveLayout; }

    void drawMenu() override;

    void addMidiEvent( const Midi_t &midiEvent ) override;

    void update( const sf::Time &deltaTime ) override;

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleOptions() const override { return m_data; }

    [[nodiscard]]
    std::deque< TimedParticle_t * > &getParticles() override { return m_particles; }

  private:
    void drawLSystem( const sf::Vector2f position,
                      const float angleDeg,
                      const int depth,
                      const Midi_t & midiNote );

  private:
    PipelineContext& m_ctx;
    LSystemCurveLayoutData_t m_data;
    ParticleBehaviorPipeline m_behaviorPipeline;

    std::deque< TimedParticle_t* > m_particles;
  };

} // namespace nx
