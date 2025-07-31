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

#include "models/IParticleBehavior.hpp"
#include "models/data/PipelineContext.hpp"

namespace nx
{
  class ParticleBehaviorPipeline final
  {
  public:

    explicit ParticleBehaviorPipeline( PipelineContext& context )
      : m_ctx( context )
    {}

    [[nodiscard]]
    nlohmann::json savePipeline() const;

    void loadPipeline( const nlohmann::json& j );

    void applyOnSpawn( IParticle * p,
                       const ParticleData_t& particleData ) const;

    void applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData ) const;

    void drawMenu();

  private:

    void drawBehaviorPipelineMenu();
    void drawBehaviorsAvailable();

    template < typename T >
    IParticleBehavior * createBehavior()
    {
      auto& behavior = m_particleBehaviors.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      return behavior.get();
    }


    template < typename T >
    IParticleBehavior * deserializeBehavior( const nlohmann::json& j )
    {
      auto& behavior = m_particleBehaviors.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_ctx ) );
      behavior->deserialize( j );
      return behavior.get();
    }

  private:

    PipelineContext& m_ctx;
    std::vector< std::unique_ptr< IParticleBehavior > > m_particleBehaviors;
  };

}