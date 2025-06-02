#pragma once

#include "models/ISerializable.hpp"
#include "models/ShaderMacros.hpp"

namespace nx
{

  class VSTParamBindingManager;

  struct IShader : public ISerializable< E_ShaderType >
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    // trigger on midi event
    virtual void trigger( const Midi_t& midi ) = 0;

    // trigger on audio buffer event (can specify certain frequencies or frequency ranges)
    virtual void trigger( const AudioDataBuffer& buffer ) = 0;

    virtual void drawMenu() = 0;

    // this is used for the shutdown process
    virtual void destroyTextures() = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture * applyShader(
      const sf::RenderTexture * inputTexture ) = 0;
  };

}