// LazyTexture.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <memory>
#include <thread>

namespace nx
{

  class LazyTexture
  {
  public:
    LazyTexture() = default;

    // this must be called from the thread that created it
    void destroy( const bool isFromDestructor = false )
    {
      if ( m_textures[ 0 ] )
      {
        // this is probably a mistake if this occurs
        if ( isFromDestructor )
        {
          LOG_WARN( "destructing thread should not destroy texture" );
        }
        ensureOwner();
        m_textures[ 0 ].reset();
        m_textures[ 1 ].reset();
      }
    }

    void ensureSize(const sf::Vector2u &size)
    {
      ensureInitialized();
      ensureOwner();

      for (auto &tex: m_textures)
      {
        if (tex->getSize() != size)
        {
          if (!tex->resize(size))
          {
            LOG_ERROR("Failed to resize render texture");
          }
        }
      }
    }

    void clear(const sf::Color &color = sf::Color::Transparent)
    {
      ensureInitialized();
      ensureOwner();
      getBack()->clear(color);
    }

    void display()
    {
      ensureInitialized();
      ensureOwner();
      getBack()->display();
      std::swap(m_frontIndex, m_backIndex); // Swap after render completes
      glFlush(); // <-- lightweight, non-blocking flush
    }

    void draw(const sf::Drawable &drawable, const sf::RenderStates &states = sf::RenderStates::Default)
    {
      ensureInitialized();
      ensureOwner();
      getBack()->draw(drawable, states);
    }

    sf::Vector2u getSize() const { return m_textures[ 0 ]->getSize(); }

    sf::RenderTexture * get() { return getFront(); }

    [[nodiscard]]
    const sf::RenderTexture * get() const { return getFront(); }

    const sf::Texture& getTexture()
    {
      return getFront()->getTexture();
    }

    bool isInitialized() const
    {
      return m_textures[ 0 ] != nullptr;
    }

  private:
    void ensureInitialized()
    {
      if (!m_textures[ 0 ])
      {
        for (auto &tex: m_textures)
        {
          tex = std::make_unique< sf::RenderTexture >();
        }
        m_ownerThreadId = std::this_thread::get_id();
      }
    }

    void ensureOwner() const
    {
      if (std::this_thread::get_id() != m_ownerThreadId)
      {
        LOG_ERROR("Texture used from wrong thread!");
      }
    }

    sf::RenderTexture *getFront() { return m_textures[ m_frontIndex ].get(); }
    sf::RenderTexture *getBack() { return m_textures[ m_backIndex ].get(); }
    const sf::RenderTexture *getFront() const { return m_textures[ m_frontIndex ].get(); }

    std::unique_ptr< sf::RenderTexture > m_textures[ 2 ];
    int m_frontIndex{ 0 };
    int m_backIndex{ 1 };

    std::thread::id m_ownerThreadId{};
  };

} // namespace nx
