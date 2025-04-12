#pragma once

namespace nx
{

  struct PerlinDeformerData_t
  {
    float noiseScale = 0.01f;     // spatial frequency
    float timeScale = 1.0f;       // temporal speed
    float deformStrength = 10.f;  // how much to offset
  };

  class PerlinDeformerModifier final : public IParticleModifier
  {
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

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Perlin Deformer" ) )
      {
        ImGui::SliderFloat("Deform Strength", &m_data.deformStrength, 0.f, 100.f);
        ImGui::SliderFloat("Noise Scale", &m_data.noiseScale, 0.001f, 0.1f, "%.4f");
        ImGui::SliderFloat("Time Speed", &m_data.timeScale, 0.f, 5.f);

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update(const sf::Time &deltaTime) override
    {
      m_time += deltaTime.asSeconds() * m_data.timeScale;
    }

    [[nodiscard]]
    sf::RenderTexture &modifyParticles(const ParticleLayoutData_t &particleLayoutData,
                                       std::deque< TimedParticle_t * > &particles) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "Failed to resize Perlin Deformer texture" );
        }
      }

      m_outputTexture.clear(sf::Color::Transparent);

      for ( const auto * p : particles )
      {
        const sf::Vector2f pos = p->shape.getPosition();
        const float x = pos.x * m_data.noiseScale;
        const float y = pos.y * m_data.noiseScale;

        const float offsetX = (noise(x + m_time, y) - 0.5f) * 2.f * m_data.deformStrength;
        const float offsetY = (noise(x, y + m_time) - 0.5f) * 2.f * m_data.deformStrength;

        sf::Vector2f warpedPos = pos + sf::Vector2f(offsetX, offsetY);

        m_outputTexture.draw(p->shape);
        // Optionally draw a ghost circle at the deformed position
        sf::CircleShape shape = p->shape;
        shape.setPosition(warpedPos);
        m_outputTexture.draw(shape);

        // Optionally draw a line from original to warped
        GradientLine line;
        line.setStart( pos );
        line.setEnd( warpedPos );
        line.setColorStart( sf::Color::White );
        line.setColorEnd( sf::Color::White );
        m_outputTexture.draw( line );
      }

      m_outputTexture.display();

      return m_outputTexture;
    }

  private:

    static float noise(float x, float y)
    {
      const float dotVal = x * 12.9898f + y * 78.233f;
      const float sinVal = std::sin(dotVal) * 43758.5453f;
      return sinVal - std::floor(sinVal); // now in [0.0, 1.0)
    }



  private:
    const GlobalInfo_t& m_globalInfo;

    PerlinDeformerData_t m_data;

    float m_time { 0.f };

    sf::RenderTexture m_outputTexture;
  };
}