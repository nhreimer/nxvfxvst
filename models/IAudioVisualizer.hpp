#pragma once

#include "models/InterfaceTypes.hpp"

namespace nx
{
  struct IAudioVisualizer : public ISerializable< E_AudioVisualizerType >
  {
    // render thread only!
    virtual sf::RenderTexture * draw( sf::RenderTexture * inTexture ) = 0;

    // render thread only!
    virtual void destroyTextures() = 0;

    // draw ImGui menu
    virtual void drawMenu() = 0;

    virtual void receiveUpdatedAudioBuffer( const AudioDataBuffer& fft ) = 0;
    virtual void update( const sf::Time& deltaTime ) = 0;
    virtual sf::BlendMode& getBlendMode() = 0;

    virtual bool isEnabled() const = 0;
    virtual void setEnabled( bool value ) = 0;
  };
}