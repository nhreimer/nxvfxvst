#pragma once

#include "models/shader/BlurShader.hpp"
#include "models/shader/KaleidoscopeShader.hpp"
#include "models/shader/DensityHeatMapShader.hpp"
#include "models/shader/DualKawaseBlurShader.hpp"
#include "models/shader/FeedbackShader.hpp"
#include "models/shader/LayeredGlitchShader.hpp"
#include "models/shader/RippleShader.hpp"
#include "models/shader/RumbleShader.hpp"
#include "models/shader/SmearShader.hpp"
#include "models/shader/StrobeShader.hpp"
#include "models/shader/TransformShader.hpp"
#include "models/shader/ColorShader.hpp"
#include "models/shader/ShockBloomShader.hpp"

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
      // TODO: add processEvent for shaders to add mouse/keyboard effects
      // TODO: have way to hide mouse icon
      //for ( auto& shader : m_shaders ) shader->processEvent( event );
    }

    void processMidiEvent( const Midi_t& midiEvent ) const
    {
      for ( auto& shader : m_shaders ) shader->trigger( midiEvent );
    }

    void drawMenu()
    {
      drawShadersAvailable();
      drawShaderPipeline();
    }

    sf::RenderTexture& draw( const sf::RenderTexture& inTexture )
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( inTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize shader pipeline texture!" );
        }
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
        createShader( SerialHelper::deserializeEnum< E_ShaderType >( type ), shaderData );
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
            shader = deserializeShader< LayeredGlitchShader >( j );
          break;

        case E_ShaderType::E_KaleidoscopeShader:
          shader = deserializeShader< KaleidoscopeShader >( j );
          break;

        case E_ShaderType::E_BlurShader:
          shader = deserializeShader< BlurShader >( j );
          break;

        case E_ShaderType::E_StrobeShader:
          shader = deserializeShader< StrobeShader >( j );
          break;

        case E_ShaderType::E_RippleShader:
          shader = deserializeShader< RippleShader >( j );
          break;

        case E_ShaderType::E_RumbleShader:
          shader = deserializeShader< RumbleShader >( j );
          break;

        case E_ShaderType::E_SmearShader:
          shader = deserializeShader< SmearShader >( j );
          break;

        case E_ShaderType::E_DensityHeatMapShader:
          shader = deserializeShader< DensityHeatMapShader >( j );
          break;

        case E_ShaderType::E_FeedbackShader:
          shader = deserializeShader< FeedbackShader >( j );
          break;

        case E_ShaderType::E_DualKawaseBlurShader:
          shader = deserializeShader< DualKawaseBlurShader >( j );
          break;

        case E_ShaderType::E_TransformShader:
          shader = deserializeShader< TransformShader >( j );
          break;

        case E_ShaderType::E_ColorShader:
          shader = deserializeShader< ColorShader >( j );
          break;

        case E_ShaderType::E_ShockBloomShader:
          shader = deserializeShader< ShockBloomShader >( j );
          break;

        default:
          LOG_ERROR( "Unsupported shader type" );
          break;
      }

      return shader;
    }

  private:

    template < typename T >
    IShader * createShader()
    {
      auto& shader = m_shaders.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_globalInfo ) );
      return shader.get();
    }

    template < typename T >
    IShader * deserializeShader( const nlohmann::json& j )
    {
      auto& shader = m_shaders.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_globalInfo ) );
      shader->deserialize( j );
      return shader.get();
    }

    void drawShadersAvailable()
    {
      if ( ImGui::TreeNode( "FX Available" ) )
      {
        ImGui::SeparatorText( "Utilities" );
        {
          if ( ImGui::Button( "Color##1" ) )
            createShader< ColorShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Feedback##1" ) )
            createShader< FeedbackShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Transform##1" ) )
            createShader< TransformShader >();
        }

        ImGui::SeparatorText( "Blur" );
        {
          if ( ImGui::Button( "Gauss Blur##1" ) )
            createShader< BlurShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "DK Blur##1" ) )
            createShader< DualKawaseBlurShader >();
        }

        ImGui::SeparatorText( "Impact" );
        {
          if ( ImGui::Button( "Glitch##1" ) )
            createShader< LayeredGlitchShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Rumble##1" ) )
            createShader< RumbleShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Strobe##1" ) )
            createShader< StrobeShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Shock Bloom##1" ) )
            createShader< ShockBloomShader >();
        }

        ImGui::SeparatorText( "Warping" );
        {
          if ( ImGui::Button( "Cosmic-Kaleido##1" ) )
            createShader< KaleidoscopeShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Density Map##1" ) )
            createShader< DensityHeatMapShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Ripple##1" ) )
            createShader< RippleShader >();

          ImGui::SameLine();
          if ( ImGui::Button( "Smear##1" ) )
            createShader< SmearShader >();
        }
        ImGui::SameLine();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void drawShaderPipeline()
    {
      ImGui::Separator();
      ImGui::Text( "Shaders: %d", m_shaders.size() );

      int deletePos = -1;
      int swapA = -1;
      int swapB = -1;

      if ( ImGui::TreeNode( "Active FX" ) )
      {
        for ( int i = 0; i < m_shaders.size(); ++i )
        {
          ImGui::PushID( i );

          if ( i > 0 )
            ImGui::Separator();

          if ( ImGui::Button( "x" ) )
            deletePos = i;
          else
          {
            ImGui::SameLine();
            m_shaders[ i ]->drawMenu();

            if ( ImGui::Button( "u" ) )
            {
              swapA = i;
              swapB = i - 1;
            }

            ImGui::SameLine();

            if ( ImGui::Button( "d" ) )
            {
              swapA = i;
              swapB = i + 1;
            }
          }
          ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( deletePos > -1 )
        m_shaders.erase( m_shaders.begin() + deletePos );
      else if ( swapA > -1 && swapB > -1 && swapA < m_shaders.size() && swapB < m_shaders.size() )
        std::swap( m_shaders[ swapA ], m_shaders[ swapB ] );
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    std::vector< std::unique_ptr< IShader > > m_shaders;
    sf::RenderTexture m_outputTexture;
  };

}