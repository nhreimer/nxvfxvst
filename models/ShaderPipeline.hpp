#pragma once

#include "data/PipelineContext.hpp"

namespace nx
{

  class RequestSink;

  class ShaderPipeline final
  {
  public:
    ///
    /// @param context
    /// @param requestSink required for requesting render-thread cleanups
    ShaderPipeline( PipelineContext& context, RequestSink& requestSink )
      : m_ctx( context ),
        m_requestSink( requestSink )
    {}

    ~ShaderPipeline() = default;

    ///////////////////////////////////////////////////////
    /// Shader actions
    ///////////////////////////////////////////////////////

    void update( const sf::Time& deltaTime ) const;

    void processEvent( const sf::Event &event ) const {}

    void processMidiEvent( const Midi_t& midiEvent ) const;

    void drawMenu();

    void destroyTextures()
    {
      std::unique_lock lock(m_mutex);
      m_outputTexture.destroy();

      // TODO: this might interfere with serialization
      for ( auto& shader : m_shaders )
      {
        shader->destroyTextures();
        shader.reset();
      }

      m_shaders.clear();
    }

    sf::RenderTexture * draw( const sf::RenderTexture * inTexture );

    ///////////////////////////////////////////////////////
    /// Shader management
    ///////////////////////////////////////////////////////

    void clear() { m_shaders.clear(); }

    void deleteShader( const int position );

    nlohmann::json saveShaderPipeline() const;
    void loadShaderPipeline( const nlohmann::json& j );
    void swapShaderPositions( const int from, const int to );

    size_t size() const { return m_shaders.size(); }

    IShader * getShader( const int position ) const;

    IShader * createShader( const E_ShaderType shaderType,
                            const nlohmann::json& j );

  private:

    template < typename T >
    IShader * createShader()
    {
      auto& shader = m_shaders.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      return shader.get();
    }

    template < typename T >
    IShader * deserializeShader( const nlohmann::json& j )
    {
      auto& shader = m_shaders.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      shader->deserialize( j );
      return shader.get();
    }

    void drawShadersAvailable();
    void drawShaderPipeline();

  private:
    PipelineContext& m_ctx;

    std::vector< std::unique_ptr< IShader > > m_shaders;
    LazyTexture m_outputTexture;

    RequestSink& m_requestSink;

    std::mutex m_mutex;
  };

}