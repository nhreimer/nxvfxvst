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

#include "models/IParticleBehavior.hpp"
#include "models/data/PipelineContext.hpp"

namespace nx
{

  class JitterBehavior final : public IParticleBehavior
  {

#define JITTER_BEHAVIOR_PARAMS(X) \
X(jitterMultiplier  , float, 0.5f, 0.f, 5.f, "Amount of jitter", true)

    struct JitterData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(JITTER_BEHAVIOR_PARAMS)
    };

    enum class E_JitterBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(JITTER_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_JitterBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(JITTER_BEHAVIOR_PARAMS)
    };

  public:
    explicit JitterBehavior( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(JITTER_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_BehaviorType getType() const override { return E_BehaviorType::E_JitterBehavior; }

    void applyOnSpawn( IParticle * p,
                       const ParticleData_t& particleData ) override;

    void applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData ) override;

    void drawMenu() override;

  private:
    sf::Vector2f getJitterPosition( const IParticle * p );

  private:
      PipelineContext& m_ctx;

      std::mt19937 m_rand;
      JitterData_t m_data;

  };

}