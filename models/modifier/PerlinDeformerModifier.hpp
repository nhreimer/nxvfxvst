#pragma once

namespace nx
{

  class PerlinDeformerModifier final : public IParticleModifier
  {

    enum class E_NoiseType : int8_t { E_Hash, E_Value, E_FBM };

#define PERLIN_DEFORMER_MODIFIER_PARAMS(X)                                                               \
X(noiseScale,        float,     0.01f,  0.0001f, 1.0f,   "Controls spatial frequency of noise",   true)  \
X(timeScale,         float,     1.0f,   0.0f,    10.0f,  "Time-based animation speed",           true)   \
X(deformStrength,    float,     10.0f,  0.0f,    100.0f, "Amount of distortion offset",          true)   \
X(colorFade,         float,     1.0f,   0.0f,    5.0f,   "Color fade strength",                  true)   \
X(octaves,           int32_t,   4,      1,      12,     "Number of FBM octaves",                 true)   \
X(useParticleColors, bool,      false,  0,      1,      "Use individual particle colors",        true)   \
X(perlinColor,       sf::Color, sf::Color(255, 255, 255, 100), 0, 255, "Fallback color",         false)

    struct PerlinDeformerData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(PERLIN_DEFORMER_MODIFIER_PARAMS)
      E_NoiseType noiseType { E_NoiseType::E_FBM };
    };

    enum class E_PerlinDeformerBehaviorParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(PERLIN_DEFORMER_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_PerlinDeformerBehaviorParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(PERLIN_DEFORMER_MODIFIER_PARAMS)
    };

  public:

    explicit PerlinDeformerModifier( PipelineContext& context )
      : m_ctx( context )
    {
      EXPAND_SHADER_VST_BINDINGS(PERLIN_DEFORMER_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ~PerlinDeformerModifier() override = default;

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_ModifierType getType() const override { return E_ModifierType::E_PerlinDeformerModifier; }

    bool isActive() const override { return m_data.isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    void drawMenu() override;

    void update(const sf::Time &deltaTime) override
    {
      m_time += ( deltaTime.asSeconds() * m_data.timeScale.first );
    }

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override;

  private:

    float getNoise( const float x, const float y ) const;

    // hash noise: fast but low quality
    static float getHashNoise(float x, float y);

    // TODO: use std::lerp instead
    static float lerp(const float a, const float b, const float t) { return a + t * (b - a); }

    // Smooth blend between grid points
    // Simple 2D value noise with bilinear interpolation
    static float getValueNoise( const float x, const float y);

    // Fractal Brownian Motion
    // Layered noise — adds complexity and control
    static float getFBM( const float x, const float y, const int octaves);


  private:
    PipelineContext& m_ctx;

    PerlinDeformerData_t m_data;

    float m_time { 0.f };
  };
}