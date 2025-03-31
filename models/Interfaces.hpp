#pragma once

#include "particle/ParticleLayoutData_t.hpp"
#include "particle/TimedParticle.hpp"


#include <deque>

namespace nx
{
  struct Midi_t
  {
    int16_t channel { 0 };
    int16_t pitch { 0 };
    float velocity { 0.f };
    sf::BlendMode blendMode { sf::BlendAdd };
  };

  struct WindowInfo_t
  {
    sf::Vector2u windowSize;
    sf::View windowView;
    bool hideMenu { false };
  };

  struct IMenuable
  {
    virtual ~IMenuable() = default;
    virtual void drawMenu() = 0;
  };

  struct IParticleLayout : public IMenuable//, public IIdentity
  {
    ~IParticleLayout() override = default;

    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;
    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual const ParticleLayoutData_t& getParticleOptions() const = 0;
    virtual std::deque< TimedParticle >& getParticles() = 0;
  };

  struct IParticleModifier : public IMenuable//, public IIdentity
  {
    ~IParticleModifier() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual sf::RenderTexture& modifyParticles(
      const ParticleLayoutData_t& particleLayoutData,
      std::deque< TimedParticle >& particles ) = 0;
  };

  struct IShader : public IMenuable//, public IIdentity
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual bool isShaderActive() const = 0;

    virtual sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) = 0;
  };

}