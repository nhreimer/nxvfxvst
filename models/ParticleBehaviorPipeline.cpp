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

#include "models/ParticleBehaviorPipeline.hpp"

#include "models/particle/behavior/FreeFallBehavior.hpp"
#include "models/particle/behavior/JitterBehavior.hpp"
#include "models/particle/behavior/MagneticBehavior.hpp"
#include "models/particle/behavior/EnergyFlowFieldBehavior.hpp"
#include "models/particle/behavior/WaveBehavior.hpp"

namespace nx
{

  nlohmann::json ParticleBehaviorPipeline::savePipeline() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( const auto& behavior : m_particleBehaviors )
      j.push_back( behavior->serialize() );

    return j;
  }

  void ParticleBehaviorPipeline::loadPipeline( const nlohmann::json& j )
  {
    m_particleBehaviors.clear();
    for ( const auto& data : j )
    {
      if ( !j.contains( "type" ) )
        continue;

      const auto type =
        SerialHelper::deserializeEnum< E_BehaviorType >( data.value("type", "" ) );
      switch ( type )
      {
        case E_BehaviorType::E_JitterBehavior:
          deserializeBehavior< JitterBehavior >( data );
          break;

        case E_BehaviorType::E_FreeFallBehavior:
          deserializeBehavior< FreeFallBehavior >( data );
          break;

        case E_BehaviorType::E_MagneticBehavior:
          deserializeBehavior< MagneticBehavior >( data );
          break;

        case E_BehaviorType::E_EnergyFlowFieldBehavior:
          deserializeBehavior< EnergyFlowFieldBehavior >( data );
          break;

        case E_BehaviorType::E_WaveBehavior:
          deserializeBehavior< WaveBehavior >( data );
          break;

        default:
          LOG_ERROR( "unable to deserialize modifier type" );
          break;
      }
    }
  }

  void ParticleBehaviorPipeline::applyOnSpawn( IParticle * p,
                                               const ParticleData_t& particleData ) const
  {
    for ( const auto& behavior : m_particleBehaviors )
      behavior->applyOnSpawn( p, particleData );
  }

  void ParticleBehaviorPipeline::applyOnUpdate( IParticle * p,
                                                const sf::Time& deltaTime,
                                                const ParticleData_t& particleData ) const
  {
    for ( const auto& behavior : m_particleBehaviors )
      behavior->applyOnUpdate( p, deltaTime, particleData );
  }

  void ParticleBehaviorPipeline::drawMenu()
  {
    drawBehaviorsAvailable();
    drawBehaviorPipelineMenu();
  }

  void ParticleBehaviorPipeline::drawBehaviorPipelineMenu()
  {
    ImGui::Separator();
    ImGui::Text( "Behaviors: %d", m_particleBehaviors.size() );

    int deletePos = -1;
    int swapA = -1;
    int swapB = -1;

    if ( ImGui::TreeNode( "Active Behaviors" ) )
    {
      for ( int i = 0; i < m_particleBehaviors.size(); ++i )
      {
        ImGui::PushID( i );

        if ( i > 0 )
          ImGui::Separator();

        if ( ImGui::Button( "x" ) )
          deletePos = i;
        else
        {
          ImGui::SameLine();
          m_particleBehaviors[ i ]->drawMenu();

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
      m_particleBehaviors.erase( m_particleBehaviors.begin() + deletePos );
    else if ( swapA > -1 && swapB > -1 && swapA < m_particleBehaviors.size() && swapB < m_particleBehaviors.size() )
      std::swap( m_particleBehaviors[ swapA ], m_particleBehaviors[ swapB ] );
  }

  void ParticleBehaviorPipeline::drawBehaviorsAvailable()
  {
    if ( ImGui::TreeNode( "Behaviors Available" ) )
    {
      if ( ImGui::Button( "Free Fall##1" ) )
        createBehavior< FreeFallBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Jitter##1" ) )
        createBehavior< JitterBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Magnetic##1" ) )
        createBehavior< MagneticBehavior >();

      if ( ImGui::Button( "Energy Flow Field##1" ) )
        createBehavior< EnergyFlowFieldBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Wave##1" ) )
        createBehavior< WaveBehavior >();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }
}