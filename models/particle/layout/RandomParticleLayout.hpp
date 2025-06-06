#pragma once

#include <random>

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

class RandomParticleLayout final : public ParticleLayoutBase< ParticleLayoutData_t >
{
public:

  explicit RandomParticleLayout( PipelineContext& context )
    : ParticleLayoutBase( context )
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override;

  void deserialize(const nlohmann::json &j) override;

  E_LayoutType getType() const override { return E_LayoutType::E_RandomLayout; }

  void drawMenu() override;

  void addMidiEvent(const Midi_t &midiEvent) override;

protected:
  sf::Vector2f getNextPosition( const Midi_t & midiNote )
  {
    return { static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.y ) };
  }

private:

  std::mt19937 m_rand;

};

}