#pragma once

namespace nx
{

  // this is an internal utility shader that gives the ability to blend
  // textures. it's not meant to be used on its own but as a component
  // in other shader controls
  class BlenderShader final
  {
    public:

    BlenderShader();

    [[nodiscard]]
    sf::RenderTexture * applyShader(const sf::RenderTexture * originalTexture,
                                  const sf::RenderTexture * effectTexture,
                                  const float mixFactor );

    void destroyTextures()
    {
      m_outputTexture.destroy();
    }

  private:

    sf::Shader m_shader;
    LazyTexture m_outputTexture;

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