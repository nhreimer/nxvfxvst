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

#include "models/particle/generator/CircleParticleGenerator.hpp"
#include "models/particle/generator/RingGenerator.hpp"

namespace nx
{

  class ParticleGeneratorManager final
  {

  public:

    explicit ParticleGeneratorManager( PipelineContext& ctx )
      : m_ctx( ctx ),
        m_particleGenerator( std::make_unique< CircleParticleGenerator >( ctx ) )
    {}

    void drawMenu()
    {
      if ( ImGui::TreeNode( "Particles Available" ) )
      {
        if ( ImGui::RadioButton( "Circles##1", E_ParticleType::E_CircleParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new CircleParticleGenerator( m_ctx ) );
        if ( ImGui::RadioButton( "Rings##1", E_ParticleType::E_RingParticle == m_particleGenerator->getType() ) )
          m_particleGenerator.reset( new RingParticleGenerator( m_ctx ) );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    IParticleGenerator * getParticleGenerator() const { return m_particleGenerator.get(); }

  private:

    PipelineContext& m_ctx;

    std::unique_ptr< IParticleGenerator > m_particleGenerator;

  };

}