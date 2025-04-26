#pragma once

namespace nx
{

  // this is an internal utility shader that gives the ability to blend
  // textures. it's not meant to be used on its own but as a component
  // in other shader controls
  class BlenderShader final
  {
    public:

    BlenderShader()
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
    sf::RenderTexture& applyShader(const sf::RenderTexture& originalTexture,
                                  const sf::RenderTexture& effectTexture,
                                  const float mixFactor )
    {
      if ( m_outputTexture.getSize() != originalTexture.getSize() )
      {
        if ( !m_outputTexture.resize( originalTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize blender texture" );
        }
        else
        {
          LOG_INFO( "resized blender texture" );
        }
      }

      m_shader.setUniform("originalTex", originalTexture.getTexture());
      m_shader.setUniform("effectTex", effectTexture.getTexture());
      m_shader.setUniform("mixFactor", mixFactor);

      m_outputTexture.clear();
      m_outputTexture.draw(sf::Sprite(originalTexture.getTexture()), &m_shader);
      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    sf::Shader m_shader;
    sf::RenderTexture m_outputTexture;

    inline static const std::string m_fragmentShader = R"(uniform sampler2D originalTex;
uniform sampler2D effectTex;
uniform float mixFactor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(originalTex, 0));
    vec4 original = texture2D(originalTex, uv);
    vec4 effect = texture2D(effectTex, uv);
    gl_FragColor = mix(original, effect, mixFactor);
})";

  };

}