#pragma once

namespace nx
{
  // UNUSED: earmarked for multithreaded rendering
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