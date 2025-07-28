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

#include "models/ShaderPipeline.hpp"

#include "models/shader/BlurShader.hpp"
#include "models/shader/ColorShader.hpp"
#include "models/shader/DensityHeatMapShader.hpp"
#include "models/shader/DualKawaseBlurShader.hpp"
#include "models/shader/FeedbackShader.hpp"
#include "models/shader/KaleidoscopeShader.hpp"
#include "models/shader/LayeredGlitchShader.hpp"
#include "models/shader/RippleShader.hpp"
#include "models/shader/RumbleShader.hpp"
#include "models/shader/ShockBloomShader.hpp"
#include "models/shader/SmearShader.hpp"
#include "models/shader/StrobeShader.hpp"
#include "models/shader/TransformShader.hpp"
#include "utils/TaskQueue.hpp"

namespace nx
{

  void ShaderPipeline::update( const sf::Time& deltaTime ) const
  {
    for ( auto& shader : m_shaders )
      shader.first->update( deltaTime );
  }

  void ShaderPipeline::processMidiEvent( const Midi_t& midiEvent ) const
  {
    for ( auto& shader : m_shaders )
      shader.first->trigger( midiEvent );
  }

  void ShaderPipeline::processAudioBuffer( const AudioDataBuffer& buffer ) const
  {
    for ( auto& shader : m_shaders )
      shader.first->trigger( buffer );
  }

  void ShaderPipeline::drawMenu()
  {
    drawShadersAvailable();
    drawShaderPipeline();
  }

  sf::RenderTexture * ShaderPipeline::draw( const sf::RenderTexture * inTexture )
  {
    m_outputTexture.ensureSize( inTexture->getSize() );

    const sf::RenderTexture * currentTexture = inTexture;

    for ( auto& shader : m_shaders )
    {
      if ( shader.first->isShaderActive() )
      {
        shader.second.startTimer();
        currentTexture = shader.first->applyShader( currentTexture );
        shader.second.stopTimerAndAddSample();
      }
    }

    m_outputTexture.clear( sf::Color::Transparent );
    m_outputTexture.draw( sf::Sprite( currentTexture->getTexture() ) );
    m_outputTexture.display();

    return m_outputTexture.get();
  }

  ///////////////////////////////////////////////////////
  /// Shader management
  ///////////////////////////////////////////////////////

  void ShaderPipeline::deleteShader( const int position )
  {
    assert( position >= 0 && position < m_shaders.size() );

    // get the unique_ptr
    auto& shader = m_shaders[ position ];

    // we do NOT want the unique_ptr to go out of scope and then
    // delete our shader. we need to manually control it.
    // the real question might be "why use unique_ptr at all" in
    // this case. the reason is that it's a good debug warning
    // in our logger if unique_ptr handles the destruction.
    auto * rawPtrShader = shader.first.release();

    // create a request task for destroying the texture and then
    // the pointer
    m_requestSink.request(
      [ rawPtrShader ]()
      {
        LOG_INFO( "Shader delete task running" );
        rawPtrShader->destroyTextures();
        delete rawPtrShader;
      } );

    // remove the empty unique_ptr from the vector now because we don't
    // want a nullptr dereference, and we don't want our UI to handle it
    m_shaders.erase( m_shaders.begin() + position );
  }

  nlohmann::json ShaderPipeline::saveShaderPipeline() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( const auto& shader : m_shaders )
      j.push_back( shader.first->serialize() );

    return j;
  }

  void ShaderPipeline::loadShaderPipeline( const nlohmann::json& j )
  {
    m_shaders.clear();
    for ( const auto& shaderData : j )
    {
      auto type = shaderData.value("type", "" );
      createShader( SerialHelper::deserializeEnum< E_ShaderType >( type ), shaderData );
    }
  }

  void ShaderPipeline::swapShaderPositions( const int from, const int to )
  {
    assert( from >= 0 && to >= 0 && from < m_shaders.size() && to < m_shaders.size() );
    std::swap( m_shaders[ from ], m_shaders[ to ] );
  }

  IShader * ShaderPipeline::getShader( const int position ) const
  {
    assert( position < m_shaders.size() );
    return m_shaders[ position ].first.get();
  }

  IShader * ShaderPipeline::createShader( const E_ShaderType shaderType,
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

  void ShaderPipeline::drawShadersAvailable()
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

  void ShaderPipeline::drawShaderPipeline()
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
          m_shaders[ i ].first->drawMenu();

          ImGui::Text( "Render Time: %0.2f", m_shaders[ i ].second.getAverage() );

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
    {
      deleteShader( deletePos );
      // m_shaders.erase( m_shaders.begin() + deletePos );
    }
    else if ( swapA > -1 && swapB > -1 && swapA < m_shaders.size() && swapB < m_shaders.size() )
      std::swap( m_shaders[ swapA ], m_shaders[ swapB ] );
  }


}