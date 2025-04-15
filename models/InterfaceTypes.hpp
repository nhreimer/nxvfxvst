#pragma once

namespace nx
{
  enum E_ShaderType : int8_t
  {
    E_InvalidShader,
    E_GlitchShader,
    E_BlurShader,
    E_PulseShader,
    E_RippleShader,
    E_StrobeShader,
    E_KaleidoscopeShader,
    E_RumbleShader,
    E_SmearShader
  };

  enum E_LayoutType : int8_t
  {
    E_EmptyLayout,
    E_RandomLayout,
    E_SpiralLayout,
    E_LissajousCurveLayout,
    E_FractalRingLayout
  };

  enum E_ModifierType : int8_t
  {
    E_InvalidModifier,
    E_SequentialModifier,
    E_FullMeshModifier,
    E_PerlinDeformerModifier,
    E_TestModifier
  };

  enum E_BehaviorType : int8_t
  {
    E_InvalidBehavior,
    E_RadialSpreaderBehavior,
    E_FreeFallBehavior,
    E_JitterBehavior,
    E_ColorMorphBehavior,
    E_MagneticBehavior
  };
}