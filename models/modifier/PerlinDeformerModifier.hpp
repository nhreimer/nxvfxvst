#pragma once

namespace nx
{

  class PerlinDeformerModifier final : public IParticleModifier
  {

    enum class E_NoiseType : int8_t { E_Hash, E_Value, E_FBM };

    struct PerlinDeformerData_t
    {
      float noiseScale = 0.01f;     // spatial frequency
      float timeScale = 1.0f;       // temporal speed
      float deformStrength = 10.f;  // how much to offset
      float colorFade = 1.f;        // how to fade the color
      E_NoiseType noiseType = E_NoiseType::E_FBM;
      int32_t octaves { 4 }; // FBM only

      bool useParticleColors { false };
      sf::Color perlinColor = sf::Color(255, 255, 255, 100);
    };

  public:

    explicit PerlinDeformerModifier( PipelineContext& context )
      : m_ctx( context )
    {}

    ~PerlinDeformerModifier() override = default;

    [[nodiscard]]
    nlohmann::json serialize() const override;

    void deserialize(const nlohmann::json &j) override;

    [[nodiscard]]
    E_ModifierType getType() const override { return E_ModifierType::E_PerlinDeformerModifier; }

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    void drawMenu() override;

    void update(const sf::Time &deltaTime) override
    {
      m_time += ( deltaTime.asSeconds() * m_data.timeScale );
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
    bool m_isActive { true };
  };
}