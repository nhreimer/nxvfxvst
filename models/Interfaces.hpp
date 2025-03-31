#pragma once

#include "models/data/ParticleLayoutData_t.hpp"
#include "models/data/TimedParticle_t.hpp"
#include "models/data/Midi_t.hpp"

#include <deque>

namespace nx
{
  struct IMenuable
  {
    virtual ~IMenuable() = default;
    virtual void drawMenu() = 0;
  };

  struct IParticleLayout : public IMenuable
  {
    ~IParticleLayout() override = default;

    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;
    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual const ParticleLayoutData_t& getParticleOptions() const = 0;

    [[nodiscard]]
    virtual std::deque< TimedParticle_t >& getParticles() = 0;
  };

  struct IParticleModifier : public IMenuable
  {
    ~IParticleModifier() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& modifyParticles(
      const ParticleLayoutData_t& particleLayoutData,
      std::deque< TimedParticle_t >& particles ) = 0;
  };

  struct IShader : public IMenuable
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) = 0;
  };

}