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

#include "models/ModifierPipeline.hpp"

#include "models/modifier/MirrorModifier.hpp"
#include "models/modifier/ParticleFullMeshLineModifier.hpp"
#include "models/modifier/ParticleSequentialLineModifier.hpp"
#include "models/modifier/PerlinDeformerModifier.hpp"
#include "models/modifier/RingZoneMeshModifier.hpp"
#include "models/modifier/KnnMeshModifier.hpp"

#include "data/PipelineContext.hpp"

namespace nx
{

  void ModifierPipeline::update( const sf::Time& deltaTime ) const
  {
    for ( auto& modifier : m_modifiers )
      modifier->update( deltaTime );
  }

  void ModifierPipeline::deleteModifier( const int position )
  {
    assert( position >= 0 && position < m_modifiers.size() );
    m_modifiers.erase( m_modifiers.begin() + position );
  }

  void ModifierPipeline::drawMenu()
  {
    drawModifiersAvailable();
    drawModifierPipelineMenu();
  }

  void ModifierPipeline::processMidiEvent( const Midi_t& midiEvent ) const
  {
    for ( auto& modifier : m_modifiers )
      modifier->processMidiEvent( midiEvent );
  }

  nlohmann::json ModifierPipeline::saveModifierPipeline() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( const auto& modifier : m_modifiers )
      j.push_back( modifier->serialize() );

    return j;
  }

  void ModifierPipeline::loadModifierPipeline( const nlohmann::json& j )
  {
    m_modifiers.clear();
    for ( const auto& modifierData : j )
    {
      if ( !j.contains( "type" ) )
        continue;

      const auto type =
        SerialHelper::deserializeEnum< E_ModifierType >( j );
      switch ( type )
      {
        case E_ModifierType::E_SequentialModifier:
          deserializeModifier< ParticleSequentialLineModifier >( modifierData );
          break;

        case E_ModifierType::E_FullMeshModifier:
          deserializeModifier< ParticleFullMeshLineModifier >( modifierData );
          break;

        case E_ModifierType::E_PerlinDeformerModifier:
          deserializeModifier< PerlinDeformerModifier >( modifierData );
          break;

        case E_ModifierType::E_RingZoneMeshModifier:
          deserializeModifier< RingZoneMeshModifier >( modifierData );
          break;

        case E_ModifierType::E_MirrorModifier:
          deserializeModifier< MirrorModifier >( modifierData );
          break;

        case E_ModifierType::E_KnnMeshModifier:
          deserializeModifier< KnnMeshModifier >( modifierData );
          break;

        default:
          LOG_ERROR( "unable to deserialize modifier type" );
          break;
      }
    }
  }

  sf::RenderTexture * ModifierPipeline::applyModifiers(
    std::deque< IParticle* >& particles,
    const sf::BlendMode& blendMode )
  {
    m_outputTexture.ensureSize( m_ctx.globalInfo.windowSize );

    std::deque< sf::Drawable* > newArtifacts;

    for ( const auto& modifier : m_modifiers )
    {
      if ( modifier->isActive() )
        modifier->modify( blendMode, particles, newArtifacts );
    }

    m_outputTexture.clear( sf::Color::Transparent );

    drawArtifacts( newArtifacts, m_blendMode );
    drawParticles( particles, blendMode );

    m_outputTexture.display();
    return m_outputTexture.get();
  }

  void ModifierPipeline::drawModifierPipelineMenu()
  {
    ImGui::Separator();
    ImGui::Text( "Modifiers: %d", m_modifiers.size() );
    ImGui::Text( "Artifacts: %d", m_artifactCount );

    int deletePos = -1;
    int swapA = -1;
    int swapB = -1;

    if ( ImGui::TreeNode( "Active Modifiers" ) )
    {
      for ( int i = 0; i < m_modifiers.size(); ++i )
      {
        ImGui::PushID( i );

        if ( i > 0 )
          ImGui::Separator();

        if ( ImGui::Button( "x" ) )
          deletePos = i;
        else
        {
          ImGui::SameLine();
          m_modifiers[ i ]->drawMenu();

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
      m_modifiers.erase( m_modifiers.begin() + deletePos );
    else if ( swapA > -1 && swapB > -1 && swapA < m_modifiers.size() && swapB < m_modifiers.size() )
      std::swap( m_modifiers[ swapA ], m_modifiers[ swapB ] );
  }

  void ModifierPipeline::drawModifiersAvailable()
  {
    if ( ImGui::TreeNode( "Mods Available" ) )
    {
      MenuHelper::drawBlendOptions( m_blendMode );
      ImGui::NewLine();

      ImGui::SeparatorText( "Line Modifiers" );
      {
        if ( ImGui::Button( "Seq Line##1" ) )
          createModifier< ParticleSequentialLineModifier >();

        ImGui::SameLine();
        if ( ImGui::Button( "Mesh Line##1" ) )
          createModifier< ParticleFullMeshLineModifier >();

        ImGui::SameLine();
        if ( ImGui::Button( "Ring Zone Mesh##3" ) )
          createModifier< RingZoneMeshModifier >();

        ImGui::SameLine();
        if ( ImGui::Button( "KNN Mesh##1" ) )
          createModifier< KnnMeshModifier >();
      }

      ImGui::SeparatorText( "Augmentation Modifiers" );
      {
        if ( ImGui::Button( "Perlin Deformer##3" ) )
          createModifier< PerlinDeformerModifier >();

        ImGui::SameLine();
        if ( ImGui::Button( "Mirror##3" ) )
          createModifier< MirrorModifier >();
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

}