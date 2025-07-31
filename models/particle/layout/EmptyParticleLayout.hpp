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

#include "helpers/SerialHelper.hpp"
#include "models/IParticleLayout.hpp"
#include "models/data/PipelineContext.hpp"

namespace nx
{

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class EmptyParticleLayout final : public IParticleLayout
  {
  public:

    explicit EmptyParticleLayout( PipelineContext& )
    {}

    [[nodiscard]] nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {}

    [[nodiscard]] E_LayoutType getType() const override { return E_LayoutType::E_EmptyLayout; }

    void update(const sf::Time &deltaTime) override {}
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Empty Layout" ) )
      {
        ImGui::Text( "No Options Available" );
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    const ParticleData_t &getParticleData() const override
    {
      return m_particleData;
    }

    [[nodiscard]]
    std::deque< IParticle * > &getParticles() override
    {
      return m_particles;
    }

  private:

    ParticleData_t m_particleData;
    std::deque< IParticle * > m_particles;
  };

}