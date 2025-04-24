#pragma once

#include "models/ISerializable.hpp"
#include "models/ShaderMacros.hpp"

namespace nx
{

  struct IShader : public ISerializable< E_ShaderType >
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual void trigger( const Midi_t& midi ) = 0;

    virtual void drawMenu() = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) = 0;
  };

}