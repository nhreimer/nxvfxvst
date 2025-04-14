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
    };

  public:

    explicit PerlinDeformerModifier( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    ~PerlinDeformerModifier() override = default;

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }
    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_ModifierType getType() const override { return E_PerlinDeformerModifier; }

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Perlin Deformer" ) )
      {
        ImGui::SliderFloat("Deform Strength", &m_data.deformStrength, 0.f, 100.f);
        ImGui::SliderFloat("Noise Scale", &m_data.noiseScale, 0.001f, 0.1f, "%.4f");
        ImGui::SliderFloat("Time Speed", &m_data.timeScale, 0.f, 5.f);
        ImGui::SliderFloat("Color fade", &m_data.colorFade, 0.f, 1.f);

        if ( m_data.noiseType == E_NoiseType::E_FBM )
          ImGui::SliderInt( "FBM Octave", &m_data.octaves, 1, 8 );

        if ( ImGui::RadioButton( "Hash##1", m_data.noiseType == E_NoiseType::E_Hash ) )
        {
          m_data.noiseType = E_NoiseType::E_Hash;
        }
        else if ( ImGui::RadioButton( "Value##1", m_data.noiseType == E_NoiseType::E_Value ) )
        {
          m_data.noiseType = E_NoiseType::E_Value;
        }
        else if ( ImGui::RadioButton( "FBM##1", m_data.noiseType == E_NoiseType::E_FBM ) )
        {
          m_data.noiseType = E_NoiseType::E_FBM;
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update(const sf::Time &deltaTime) override
    {
      m_time += ( deltaTime.asSeconds() * m_data.timeScale );
    }

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override
    {
      for (size_t i = 0; i < particles.size(); ++i)
      {
        const sf::Vector2f pos = particles[ i ]->shape.getPosition();
        const float x = pos.x * m_data.noiseScale;
        const float y = pos.y * m_data.noiseScale;

        const float offsetX = (getNoise(x + m_time, y) - 0.5f) * 2.f * m_data.deformStrength;
        const float offsetY = (getNoise(x, y + m_time) - 0.5f) * 2.f * m_data.deformStrength;

        const sf::Vector2f warpedPos = pos + sf::Vector2f(offsetX, offsetY);

        // don't get stuck in an infinite loop
        //auto *copiedParticle = particles.emplace_back(new TimedParticle_t(*particles[ i ]));
        //copiedParticle->shape.setPosition(warpedPos);
        auto * copiedShape = dynamic_cast< sf::CircleShape * >(
          outArtifacts.emplace_back( new sf::CircleShape( particles[ i ]->shape ) ) );

        copiedShape->setPosition( warpedPos );
        const auto color = copiedShape->getFillColor();
        copiedShape->setFillColor( { color.r, color.g, color.b, static_cast< uint8_t >(color.a * m_data.colorFade) } );
      }
    }

  private:

    float getNoise( const float x, const float y ) const
    {
      switch ( m_data.noiseType )
      {
        case E_NoiseType::E_Hash:  return getHashNoise(x, y);
        case E_NoiseType::E_Value: return getValueNoise(x, y);
        case E_NoiseType::E_FBM:   return getFBM(x, y, m_data.octaves);
        default: return 0.f;
      }
    }

    // hash noise: fast but low quality
    static float getHashNoise(float x, float y)
    {
      const float dotVal = x * 12.9898f + y * 78.233f;
      const float sinVal = std::sin(dotVal) * 43758.5453f;
      return sinVal - std::floor(sinVal); // now in [0.0, 1.0)
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Smooth blend between grid points
    // Simple 2D value noise with bilinear interpolation
    static float getValueNoise( const float x, const float y)
    {
      const int xi = static_cast<int>(std::floor(x));
      const int yi = static_cast<int>(std::floor(y));
      const float xf = x - xi;
      const float yf = y - yi;

      const float tl = getHashNoise(xi, yi);
      const float tr = getHashNoise(xi + 1, yi);
      const float bl = getHashNoise(xi, yi + 1);
      const float br = getHashNoise(xi + 1, yi + 1);

      const float top = lerp(tl, tr, xf);
      const float bottom = lerp(bl, br, xf);

      return lerp(top, bottom, yf); // [0,1]
    }

    // Fractal Brownian Motion
    // Layered noise — adds complexity and control
    static float getFBM( const float x, const float y, const int octaves)
    {
      float value = 0.0f;
      float amplitude = 0.5f;
      float frequency = 1.0f;

      for (int i = 0; i < octaves; ++i)
      {
        value += amplitude * getValueNoise(x * frequency, y * frequency);
        frequency *= 2.0f;
        amplitude *= 0.5f;
      }

      return value;
    }


  private:
    const GlobalInfo_t& m_globalInfo;

    PerlinDeformerData_t m_data;

    float m_time { 0.f };
    bool m_isActive { true };
  };
}