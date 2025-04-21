#pragma once

namespace nx
{
  struct ScopedRenderContext
  {
    sf::RenderTexture& texture;

    explicit ScopedRenderContext( sf::RenderTexture& t )
      : texture( t )
    {
      t.setActive( true );
    }

    ~ScopedRenderContext()
    {
      texture.setActive( false );
    }
  };
}