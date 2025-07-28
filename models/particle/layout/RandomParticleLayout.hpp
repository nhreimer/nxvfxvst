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

#include <random>

#include "models/particle/layout/ParticleLayoutBase.hpp"

namespace nx
{

class RandomParticleLayout final : public ParticleLayoutBase< ParticleLayoutData_t >
{
public:

  explicit RandomParticleLayout( PipelineContext& context )
    : ParticleLayoutBase( context )
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override;

  void deserialize(const nlohmann::json &j) override;

  E_LayoutType getType() const override { return E_LayoutType::E_RandomLayout; }

  void drawMenu() override;

  void addMidiEvent(const Midi_t &midiEvent) override;

protected:
  sf::Vector2f getNextPosition( const Midi_t & midiNote )
  {
    return { static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.y ) };
  }

private:

  std::mt19937 m_rand;

};

}