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

namespace nx
{
  enum class E_ShaderType : int8_t
  {
    E_InvalidShader,
    E_GlitchShader,
    E_BlurShader,
    E_RippleShader,
    E_StrobeShader,
    E_KaleidoscopeShader,
    E_RumbleShader,
    E_SmearShader,
    E_DensityHeatMapShader,
    E_FeedbackShader,
    E_DualKawaseBlurShader,
    E_TransformShader,
    E_ColorShader,
    E_ShockBloomShader
  };

  enum class E_LayoutType : int8_t
  {
    E_EmptyLayout,
    E_RandomLayout,
    E_SpiralLayout,
    E_LissajousCurveLayout,
    E_FractalRingLayout,
    E_LSystemCurveLayout,
    E_GoldenSpiralLayout,
    E_EllipticalLayout,

    // the bottom two are the only "real" ones
    //E_RingPlotVisualizer,
    E_SpiralEchoVisualizer,
    E_RingParticleVisualizer,
    E_PlotLineVisualizer,
    E_TessellationVisualizer,
    E_VortexSinkVisualizer,

    E_TestLayout
  };

  enum class E_ModifierType : int8_t
  {
    E_InvalidModifier,
    E_SequentialModifier,
    E_FullMeshModifier,
    E_PerlinDeformerModifier,
    E_RingZoneMeshModifier,
    E_MirrorModifier,
    E_KnnMeshModifier,
    E_TestModifier
  };

  enum class E_BehaviorType : int8_t
  {
    E_InvalidBehavior,
    E_FreeFallBehavior,
    E_JitterBehavior,
    E_MagneticBehavior,
    E_EnergyFlowFieldBehavior,
    E_WaveBehavior
  };

  enum class E_ParticleType : int8_t
  {
    E_InvalidParticle,
    E_CircleParticle,
    E_StarburstParticle,
    E_BurstRingParticle,
    E_RingParticle
  };
}