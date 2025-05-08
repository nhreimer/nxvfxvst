#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <thread>

namespace nx
{

  ///
  /// Lazy initialization of the sf::RenderTexture, so that any time the texture is operated
  /// on it gets initialized. This allows LazyTexture to be declared but for threads that
  /// need to use it to take ownership of it.
  ///
  /// It uses a double-buffer strategy along with glFlush() to allow OpenGL to update
  /// the screen. As a consequence, two textures are always allocated. The alternative
  /// would be to use glFinish(), which is not an optimal solution.
  class LazyTexture final
  {
  public:
    LazyTexture() = default;

    // this must be called from the thread that created it
    void destroy( const bool isFromDestructor = false );

    void ensureSize(const sf::Vector2u &size);

    void clear(const sf::Color &color = sf::Color::Transparent);

    void display();

    void draw(const sf::Drawable &drawable,
              const sf::RenderStates &states = sf::RenderStates::Default);

    [[nodiscard]]
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
    void ensureInitialized();

    void ensureOwner() const
    {
      if (std::this_thread::get_id() != m_ownerThreadId)
      {
        LOG_ERROR("Texture used from wrong thread!");
      }
    }

    sf::RenderTexture *getFront() { return m_textures[ m_frontIndex ].get(); }
    sf::RenderTexture *getBack() { return m_textures[ m_backIndex ].get(); }

    [[nodiscard]]
    const sf::RenderTexture *getFront() const { return m_textures[ m_frontIndex ].get(); }

    std::unique_ptr< sf::RenderTexture > m_textures[ 2 ];
    int m_frontIndex{ 0 };
    int m_backIndex{ 1 };

    std::thread::id m_ownerThreadId{};
  };

} // namespace nx
