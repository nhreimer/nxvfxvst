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

#include "models/InterfaceTypes.hpp"
#include "models/IParticleBehavior.hpp"
#include "models/ShaderMacros.hpp"
#include "models/data/PipelineContext.hpp"

namespace nx
{
  class FreeFallBehavior final : public IParticleBehavior
  {

#define FREE_FALL_BEHAVIOR_PARAMS(X)                                            \
X(invert,       bool ,  false,  0  ,    0,    "Reverses the direction" , true ) \
X(scrollRate,   float,  1.f,    0.f,    50.f, "Rate of movement"       , true )

    struct FreeFallData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(FREE_FALL_BEHAVIOR_PARAMS)
    };

    enum class E_FreeFallBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(FREE_FALL_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_FreeFallBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(FREE_FALL_BEHAVIOR_PARAMS)
    };

  public:
    explicit FreeFallBehavior(PipelineContext& context)
      : m_ctx(context)
    {
      EXPAND_SHADER_VST_BINDINGS(FREE_FALL_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    E_BehaviorType getType() const override { return E_BehaviorType::E_FreeFallBehavior; }

    void applyOnSpawn( IParticle * p,
                       const ParticleData_t& particleData ) override {}

    void applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData ) override;

    void drawMenu() override;

  private:
    PipelineContext& m_ctx;
    FreeFallData_t m_data;
  };
}