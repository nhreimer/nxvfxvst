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

#include "helpers/MathHelper.hpp"

namespace nx
{

  class WaveBehavior final : public IParticleBehavior
  {

#define WAVE_BEHAVIOR_PARAMS(X) \
X(waveFrequency,   float, 0.02f, 0.f, 2.0f,  "Wave frequency (Y-based)", true) \
X(waveAmplitude,   float, 30.f,  0.f, 100.f, "Wave amplitude", true)           \
X(waveSpeed,       float, 1.f,   0.f, 25.f,  "Wave scroll speed", true)        \
X(wavePhaseOffset, float, 0.f,   0.f, NX_TAU, "Wave phase shift", true)

    struct WaveData_t
    {
      EXPAND_SHADER_PARAMS_FOR_STRUCT(WAVE_BEHAVIOR_PARAMS)
    };

    enum class E_WaveBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(WAVE_BEHAVIOR_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_WaveBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(WAVE_BEHAVIOR_PARAMS)
    };

  public:
    explicit WaveBehavior( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(WAVE_BEHAVIOR_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(WAVE_BEHAVIOR_PARAMS)
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(WAVE_BEHAVIOR_PARAMS)
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    E_BehaviorType getType() const override { return E_BehaviorType::E_WaveBehavior; }

    void applyOnSpawn( IParticle * p,
                       const ParticleData_t& particleData ) override
    {
      // apply( p );
    }

    void applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData ) override
    {
      apply( p );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Wave Behavior" ) )
      {
        EXPAND_SHADER_IMGUI(WAVE_BEHAVIOR_PARAMS, m_data)
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    void apply( IParticle * p )
    {
      sf::Vector2f pos = p->getPosition();
      const float y = pos.y;

      const float time = m_ctx.globalInfo.elapsedTimeSeconds;
      const float phase = y * m_data.waveFrequency.first + time * m_data.waveSpeed.first;
      const float offset = std::sin(phase + m_data.wavePhaseOffset.first) * m_data.waveAmplitude.first;

      pos.x += offset;
      p->setPosition(pos);
    }

  private:
    PipelineContext& m_ctx;

    std::mt19937 m_rand;
    WaveData_t m_data;

  };

}