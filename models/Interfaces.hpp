#pragma once

#include <SFML/Graphics/RenderTexture.hpp>

#include "models/data/ParticleLayoutData_t.hpp"
#include "models/data/TimedParticle_t.hpp"
#include "models/data/Midi_t.hpp"

#include <deque>
#include <nlohmann/json.hpp>

namespace nx
{

  enum E_ShaderType : uint8_t
  {
    E_InvalidShader,
    E_GlitchShader,
    E_BlurShader,
    E_PulseShader,
    E_RippleShader,
    E_StrobeShader,
    E_KaleidoscopeShader,
    E_RumbleShader
  };

  enum E_LayoutType : uint8_t
  {
    E_EmptyLayout,
    E_RandomLayout,
    E_SpiralLayout
  };

  enum E_ModifierType : uint8_t
  {
    E_NoModifier,
    E_SequentialModifier,
    E_FullMeshModifier
  };

  template < typename T >
  struct ISerializable
  {
    virtual ~ISerializable() = default;

    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize( const nlohmann::json& j ) = 0;

    // identify type for easier loading
    virtual T getType() const = 0;
  };

  struct IMenuable
  {
    virtual ~IMenuable() = default;
    virtual void drawMenu() = 0;
  };

  struct IParticleLayout : public IMenuable,
                           public ISerializable< E_LayoutType >
  {
    ~IParticleLayout() override = default;

    virtual void addMidiEvent( const Midi_t& midiEvent ) = 0;
    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual const ParticleLayoutData_t& getParticleOptions() const = 0;

    [[nodiscard]]
    virtual std::deque< TimedParticle_t >& getParticles() = 0;
  };

  struct IParticleModifier : public IMenuable,
                             public ISerializable< E_ModifierType >
  {
    ~IParticleModifier() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& modifyParticles(
      const ParticleLayoutData_t& particleLayoutData,
      std::deque< TimedParticle_t >& particles ) = 0;
  };

  struct IShader : public IMenuable,
                   public ISerializable< E_ShaderType >
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual void trigger( const Midi_t& midi ) = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) = 0;
  };

}