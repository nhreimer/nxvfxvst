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

#include "particle/layout/EmptyParticleLayout.hpp"

#include "data/PipelineContext.hpp"

namespace nx
{

  class ParticleLayoutManager final
  {
  public:

    explicit ParticleLayoutManager( PipelineContext& context )
      : m_ctx( context ),
        m_particleLayout( std::make_unique< EmptyParticleLayout >( context ) )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const;
    void deserialize( const nlohmann::json& j ) const;
    void update( const sf::Time& deltaTime ) const;

    void processMidiEvent( const Midi_t& midiEvent ) const;

    void processAudioBuffer( const IFFTResult& fftResult ) const;

    [[nodiscard]]
    std::deque< IParticle* >& getParticles() const;

    void drawAudioMenu();

    void drawMidiMenu();

  private:

    // save settings between changes to make editing less frustrating
    template < typename T >
    void changeLayout()
    {
      m_tempSettings[ SerialHelper::serializeEnum( m_particleLayout->getType() ) ] = m_particleLayout->serialize();
      m_particleLayout.reset( new T( m_ctx ) );

      const auto newLayoutName = SerialHelper::serializeEnum( m_particleLayout->getType() );
      if ( m_tempSettings.contains( newLayoutName ) )
        m_particleLayout->deserialize( m_tempSettings[ newLayoutName ] );
    }

    template < typename T >
    void selectParticleLayout( const std::string& name, const E_LayoutType layoutType )
    {
      if ( ImGui::RadioButton( name.c_str(), m_particleLayout->getType() == layoutType ) )
        changeLayout< T >();
    }

  private:
    PipelineContext& m_ctx;
    std::unique_ptr< IParticleLayout > m_particleLayout;
    nlohmann::json m_tempSettings;
  };

}