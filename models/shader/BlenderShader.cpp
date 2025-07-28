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

#include "models/shader/BlenderShader.hpp"

namespace nx
{

  BlenderShader::BlenderShader()
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load blender fragment shader" );
    }
    else
    {
      LOG_INFO( "Blender shader loaded successfully" );
    }
  }

  [[nodiscard]]
  sf::RenderTexture * BlenderShader::applyShader(const sf::RenderTexture * originalTexture,
                                const sf::RenderTexture * effectTexture,
                                const float mixFactor )
  {
    m_outputTexture.ensureSize( originalTexture->getSize() );

    m_shader.setUniform("originalTex", originalTexture->getTexture());
    m_shader.setUniform("effectTex", effectTexture->getTexture());
    m_shader.setUniform("mixFactor", mixFactor);

    m_outputTexture.clear();
    m_outputTexture.draw(sf::Sprite(originalTexture->getTexture()), &m_shader);
    m_outputTexture.display();

    return m_outputTexture.get();
  }

};
