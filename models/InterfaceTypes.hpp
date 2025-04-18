#pragma once

namespace nx
{
  enum class E_ShaderType : int8_t
  {
    E_InvalidShader,
    E_GlitchShader,
    E_BlurShader,
    E_PulseShader,
    E_RippleShader,
    E_StrobeShader,
    E_KaleidoscopeShader,
    E_RumbleShader,
    E_SmearShader,
    E_DensityHeatMapShader,
    E_FeedbackShader,
    E_DualKawaseBlurShader
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

    E_TestLayout
  };

  enum class E_ModifierType : int8_t
  {
    E_InvalidModifier,
    E_SequentialModifier,
    E_FullMeshModifier,
    E_PerlinDeformerModifier,
    E_RingZoneMeshModifier,

    E_TestModifier
  };

  enum class E_BehaviorType : int8_t
  {
    E_InvalidBehavior,
    E_RadialSpreaderBehavior,
    E_FreeFallBehavior,
    E_JitterBehavior,
    E_ColorMorphBehavior,
    E_MagneticBehavior
  };
}