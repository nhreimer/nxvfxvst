/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "utils/LazyTexture.hpp"

#include <SFML/OpenGL.hpp>

namespace nx
{
  void LazyTexture::destroy( const bool isFromDestructor )
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

  void LazyTexture::ensureSize(const sf::Vector2u &size)
  {
    ensureInitialized();
    ensureOwner();

    for (const auto &tex: m_textures)
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

  void LazyTexture::clear(const sf::Color &color)
  {
    ensureInitialized();
    ensureOwner();
    getBack()->clear(color);
  }

  void LazyTexture::display()
  {
    ensureInitialized();
    ensureOwner();
    getBack()->display();
    std::swap(m_frontIndex, m_backIndex); // Swap after render completes
    glFlush(); // <-- lightweight, non-blocking flush
  }

  void LazyTexture::draw(const sf::Drawable &drawable, const sf::RenderStates &states)
  {
    ensureInitialized();
    ensureOwner();
    getBack()->draw(drawable, states);
  }

  void LazyTexture::ensureInitialized()
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

}