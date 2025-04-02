#pragma once

namespace nx
{

  class ShaderPipeline final
  {
  public:

    explicit ShaderPipeline( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    ~ShaderPipeline() = default;

    ///////////////////////////////////////////////////////
    /// Shader actions
    ///////////////////////////////////////////////////////

    void update( const sf::Time& deltaTime ) const
    {
      for ( auto& shader : m_shaders ) shader->update( deltaTime );
    }

    void processEvent( const sf::Event &event ) const
    {
      for ( auto& shader : m_shaders ) processEvent( event );
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      for ( auto& shader : m_shaders ) shader->trigger( midiEvent );
    }

    void drawMenu()
    {
      for ( auto& shader : m_shaders ) shader->drawMenu();
    }

    sf::RenderTexture& draw( const sf::RenderTexture& inTexture )
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        assert( m_outputTexture.resize( inTexture.getSize() ) );
        LOG_INFO( "Resized shader pipline texture" );
      }

      const sf::RenderTexture * currentTexture = &inTexture;

      for ( const auto& shader : m_shaders )
      {
        if ( shader->isShaderActive() )
          currentTexture = &shader->applyShader( *currentTexture );
      }

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( currentTexture->getTexture() ) );
      m_outputTexture.display();

      return m_outputTexture;
    }

    ///////////////////////////////////////////////////////
    /// Shader management
    ///////////////////////////////////////////////////////

    void clear() { m_shaders.clear(); }

    void deleteShader( const int position )
    {
      assert( position >= 0 && position < m_shaders.size() );
      m_shaders.erase( m_shaders.begin() + position );
    }

    nlohmann::json saveShaderPipeline() const
    {
      nlohmann::json j = nlohmann::json::array();

      for ( const auto& shader : m_shaders )
        j.push_back( shader->serialize() );

      return j;
    }

    void loadShaderPipeline( const nlohmann::json& j )
    {
      m_shaders.clear();
      for ( const auto& shaderData : j )
      {
        auto type = shaderData.value("type", "" );
        createShader( SerialHelper::convertStringToShaderType( type ), shaderData );
      }
    }

    void swapShaderPositions( const int from, const int to )
    {
      assert( from >= 0 && to >= 0 && from < m_shaders.size() && to < m_shaders.size() );
      std::swap( m_shaders[ from ], m_shaders[ to ] );
    }

    size_t size() const { return m_shaders.size(); }

    IShader * getShader( const int position ) const
    {
      assert( position < m_shaders.size() );
      return m_shaders[ position ].get();
    }

    IShader * createShader( const E_ShaderType shaderType,
                            const nlohmann::json& j )
    {
      IShader * shader = nullptr;

      switch ( shaderType )
      {
        case E_ShaderType::E_GlitchShader:
          shader = addShader< GlitchShader >( j );
          break;

        case E_ShaderType::E_PulseShader:
          shader = addShader< PulseShader >( j );
          break;

        case E_ShaderType::E_KaleidoscopeShader:
          shader = addShader< KaleidoscopeShader >( j );
          break;

        case E_ShaderType::E_BlurShader:
          shader = addShader< BlurShader >( j );
          break;

        case E_ShaderType::E_StrobeShader:
          shader = addShader< StrobeShader >( j );
          break;

        case E_ShaderType::E_RippleShader:
          shader = addShader< RippleShader >( j );
          break;

        default:
          LOG_ERROR( "Unsupported shader type" );
          break;
      }

      return shader;
    }

  private:

    template < typename T >
    IShader * addShader( const nlohmann::json& j )
    {
      auto& shader = m_shaders.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_globalInfo ) );
      shader->deserialize( j );
      return shader.get();
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    std::vector< std::unique_ptr< IShader > > m_shaders;

    sf::RenderTexture m_outputTexture;
  };

}