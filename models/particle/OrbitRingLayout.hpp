#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

struct OrbitRingData_t
{
  // base particle data
  ParticleLayoutData_t particleData;

  float orbitRadius { 0.f };
  float angleOffset { 0.f };
};

class OrbitRingLayout final : public IParticleLayout
{
public:
  nlohmann::json serialize() const override
  {
  }
  void deserialize(const nlohmann::json &j) override;
  E_LayoutType getType() const override;
  void drawMenu() override;
  void addMidiEvent(const Midi_t &midiEvent) override;
  void update(const sf::Time &deltaTime) override;
  [[nodiscard]] const ParticleLayoutData_t &getParticleOptions() const override;
  [[nodiscard]] std::deque< TimedParticle_t > &getParticles() override;

private:

  const GlobalInfo_t& m_globalInfo;
  std::deque< TimedParticle_t > m_particles;
  OrbitRingData_t m_data;
};

}