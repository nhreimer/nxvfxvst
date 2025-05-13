#pragma once

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "models/InterfaceTypes.hpp"

#include "models/particle/generator/ParticleGeneratorBase.hpp"

#include "models/data/ParticleData_t.hpp"
#include "models/data/Midi_t.hpp"

#include "shapes/CircleParticle.hpp"

namespace nx
{
  // class CircleParticleGenerator final : public ParticleGeneratorBase< ParticleData_t >
  // {
  // public:
  //
  //   [[nodiscard]]
  //   E_ParticleType getType() const override { return E_ParticleType::E_CircleParticle; }
  //
  //   [[nodiscard]]
  //   IParticle * createParticle( const Midi_t& midiEvent, const float timeStampInSeconds ) override
  //   {
  //     auto * particle = new CircleParticle( getData(), timeStampInSeconds );
  //     initialize( particle, midiEvent, timeStampInSeconds );
  //     return particle;
  //   }
  // };
}