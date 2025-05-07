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
