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

#include "models/ParticleLayoutManager.hpp"

#include "models/particle/layout/EmptyParticleLayout.hpp"
#include "models/particle/layout/SpiralParticleLayout.hpp"
#include "models/particle/layout/RandomParticleLayout.hpp"

#include "models/particle/layout/LSystemCurveLayout.hpp"
#include "models/particle/layout/LissajousCurveLayout.hpp"
#include "models/particle/layout/EllipticalLayout.hpp"
#include "models/particle/layout/FractalRingLayout.hpp"
#include "models/particle/layout/GoldenSpiralLayout.hpp"

#include "models/particle/layout/RingParticleVisualizer.hpp"
#include "models/particle/layout/SpiralEchoVisualizer.hpp"
#include "models/particle/layout/PlotLineVisualizer.hpp"
#include "models/particle/layout/TessellatedGridVisualizer.hpp"
#include "models/particle/layout/VortexSinkVisualizer.hpp"

namespace nx
{

  nlohmann::json ParticleLayoutManager::serialize() const
  {
    return m_particleLayout->serialize();
  }

  void ParticleLayoutManager::deserialize( const nlohmann::json& j ) const
  {
    m_particleLayout->deserialize( j );
  }

  void ParticleLayoutManager::update( const sf::Time& deltaTime ) const
  {
    m_particleLayout->update( deltaTime );
  }

  void ParticleLayoutManager::processMidiEvent(const Midi_t &midiEvent) const
  {
    m_particleLayout->addMidiEvent(midiEvent);
  }

  void ParticleLayoutManager::processAudioBuffer(const IFFTResult &fftResult) const
  {
    m_particleLayout->processAudioBuffer( fftResult );
  }

  std::deque< IParticle* >& ParticleLayoutManager::getParticles() const
  {
    return m_particleLayout->getParticles();
  }

  void ParticleLayoutManager::drawAudioMenu()
  {
    if ( ImGui::TreeNode( "Layouts Available" ) )
    {
      selectParticleLayout< EmptyParticleLayout >( "Empty", E_LayoutType::E_EmptyLayout );
      selectParticleLayout< RingParticleVisualizer >( "Ring Visualizer", E_LayoutType::E_RingParticleVisualizer );
      selectParticleLayout< SpiralEchoVisualizer >( "Spiral Echo Visualizer", E_LayoutType::E_SpiralEchoVisualizer );
      selectParticleLayout< PlotLineVisualizer >( "Plot Line Visualizer", E_LayoutType::E_PlotLineVisualizer );
      selectParticleLayout< TessellatedGridVisualizer >( "Tessellation Grid Visualizer", E_LayoutType::E_TessellationVisualizer );
      selectParticleLayout< VortexSinkVisualizer >( "Vortex Sink", E_LayoutType::E_VortexSinkVisualizer );

      ImGui::TreePop();
      ImGui::Spacing();
    }

    m_particleLayout->drawMenu();
  }


  void ParticleLayoutManager::drawMidiMenu()
  {
    if ( ImGui::TreeNode( "Layouts Available" ) )
    {
      selectParticleLayout< EmptyParticleLayout >( "Empty", E_LayoutType::E_EmptyLayout );

      ImGui::SameLine();
      selectParticleLayout< RandomParticleLayout >( "Random", E_LayoutType::E_RandomLayout );

      ImGui::SeparatorText( "Curved Layouts" );
      selectParticleLayout< SpiralParticleLayout >( "Spiral", E_LayoutType::E_SpiralLayout );

      ImGui::SameLine();
      selectParticleLayout< LissajousCurveLayout >( "Lissajous Curve", E_LayoutType::E_LissajousCurveLayout );

      ImGui::SameLine();
      selectParticleLayout< GoldenSpiralLayout >( "Golden Spiral", E_LayoutType::E_GoldenSpiralLayout );

      ImGui::SameLine();
      selectParticleLayout< EllipticalLayout >( "Elliptical", E_LayoutType::E_EllipticalLayout );

      ImGui::SeparatorText( "Fractal Layouts" );
      selectParticleLayout< FractalRingLayout >( "Fractal Ring", E_LayoutType::E_FractalRingLayout );

      ImGui::SameLine();
      selectParticleLayout< LSystemCurveLayout >( "L-System Curve", E_LayoutType::E_LSystemCurveLayout );

      ImGui::TreePop();
      ImGui::Spacing();
    }

    ImGui::Separator();
    m_particleLayout->drawMenu();
  }

}