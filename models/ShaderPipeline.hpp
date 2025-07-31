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

#pragma once

#include <mutex>

#include "models/InterfaceTypes.hpp"
#include "models/IShader.hpp"
#include "models/data/Midi_t.hpp"
#include "models/data/PipelineContext.hpp"
#include "utils/LazyTexture.hpp"
#include "utils/RingBufferAverager.hpp"

namespace nx
{

  class RequestSink;

  class ShaderPipeline final
  {

    using ShaderPair = std::pair< std::unique_ptr< IShader >, RingBufferAverager >;

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

    void processAudioBuffer( const AudioDataBuffer& buffer ) const;

    void drawMenu();

    void destroyTextures()
    {
      std::unique_lock lock(m_mutex);
      m_outputTexture.destroy();

      // TODO: this might interfere with serialization
      for ( auto& shader : m_shaders )
      {
        shader.first->destroyTextures();
        shader.first.reset();
      }

      m_shaders.clear();
    }

    sf::RenderTexture * draw( const sf::RenderTexture * inTexture );

    ///////////////////////////////////////////////////////
    /// Shader management
    ///////////////////////////////////////////////////////

    void clear() { m_shaders.clear(); }

    void deleteShader( int position );

    [[nodiscard]]
    nlohmann::json saveShaderPipeline() const;

    void loadShaderPipeline( const nlohmann::json& j );
    void swapShaderPositions( int from, int to );

    [[nodiscard]]
    size_t size() const { return m_shaders.size(); }

    [[nodiscard]]
    IShader * getShader( int position ) const;

    IShader * createShader( E_ShaderType shaderType,
                            const nlohmann::json& j );

  private:

    template < typename T >
    IShader * createShader()
    {
      auto& shader =
        m_shaders.emplace_back( std::pair< std::unique_ptr< T >, RingBufferAverager >( { std::make_unique< T >( m_ctx ),
                                  RingBufferAverager {} } ) );
      return shader.first.get();
    }

    template < typename T >
    IShader * deserializeShader( const nlohmann::json& j )
    {
      auto& shader =
        m_shaders.emplace_back( std::pair< std::unique_ptr< T >, RingBufferAverager >( { std::make_unique< T >( m_ctx ),
                                  RingBufferAverager {} } ) );

      shader.first->deserialize( j );
      return shader.first.get();
    }

    void drawShadersAvailable();
    void drawShaderPipeline();

  private:
    PipelineContext& m_ctx;

    //std::vector< std::unique_ptr< IShader > > m_shaders;
    std::vector< ShaderPair > m_shaders;
    LazyTexture m_outputTexture;

    RequestSink& m_requestSink;

    std::mutex m_mutex;
  };

}