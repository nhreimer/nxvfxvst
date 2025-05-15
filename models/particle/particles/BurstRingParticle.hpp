#pragma once

namespace nx
{

struct BurstRingParticleData_t : public ParticleData_t
{
  int32_t spokes { 4 }; // 4 - 64
  float spokeLengthMultiplier { 1.25f };
  float spokeThickness { 2.f };
  float innerRadius { 0.f };
};

class BurstRingParticle final : public TimedParticleBase
{
public:

  BurstRingParticle( const BurstRingParticleData_t& data,
                     const float timeStamp )
    : m_data( data ),
      m_radiusOverride( m_data.radius )
  {
    m_spawnTimeInSeconds = timeStamp;
    updateShapes();
  }

  BurstRingParticle( const BurstRingParticleData_t& data,
                     const float timeStamp,
                     const float radius )
    : m_data( data ),
      m_radiusOverride( radius )
  {
    m_spawnTimeInSeconds = timeStamp;
    updateShapes();
  }

  IParticle* clone(const float timeStampInSeconds) const override
  {
    auto* cloned = new BurstRingParticle(m_data, timeStampInSeconds);
    cloned->setColorPattern(m_data.fillStartColor, m_data.fillEndColor);
    cloned->setOutlineColorPattern(m_data.outlineStartColor, m_data.outlineEndColor);
    return cloned;
  }

  uint8_t getPointCount() const override { return 0; } // not meaningful here
  float getOutlineThickness() const override { return m_ring.getOutlineThickness(); }
  float getRadius() const override { return m_radiusOverride; }

  sf::FloatRect getLocalBounds() const override
  {
    return m_ring.getLocalBounds();
  }

  sf::FloatRect getGlobalBounds() const override
  {
    return getTransform().transformRect(m_ring.getGlobalBounds());
  }

  void setColorPattern(const sf::Color& startColor, const sf::Color& endColor) override
  {
    m_ring.setFillColor(startColor);
    for (auto& spoke : m_spokes)
      spoke.setFillColor(startColor);
  }

  std::pair<sf::Color, sf::Color> getColors() const override
  {
    return { m_data.fillStartColor, m_data.fillEndColor };
  }

  void setOutlineColorPattern(const sf::Color& startColor, const sf::Color& endColor) override
  {
    m_ring.setOutlineColor(startColor);
  }

  std::pair<sf::Color, sf::Color> getOutlineColors() const override
  {
    return { m_data.outlineStartColor, m_data.outlineEndColor };
  }

  void draw(sf::RenderTarget& target, sf::RenderStates states) const override
  {
    states.transform *= getTransform();
    target.draw(m_donutRing, states);
    for (const auto& spoke : m_spokes)
      target.draw(spoke, states);
  }

private:
  void updateShapes()
  {
    updateDonutRing();
    updateSpokes();
  }

  void updateSpokes()
  {
    const float spokeLen = m_radiusOverride * m_data.spokeLengthMultiplier;

    m_ring.setRadius(m_radiusOverride);
    m_ring.setOrigin({m_radiusOverride, m_radiusOverride});
    m_ring.setPointCount(64);
    m_ring.setOutlineThickness(m_data.outlineThickness);
    m_ring.setOutlineColor(m_data.outlineStartColor);
    m_ring.setFillColor(m_data.fillStartColor);

    m_spokes.clear();
    for (uint8_t i = 0; i < m_data.spokes; ++i)
    {
      const float angle = (NX_TAU * i) / m_data.spokes;

      auto& spoke = m_spokes.emplace_back();
      spoke.setSize({ spokeLen, m_data.spokeThickness });
      spoke.setOrigin({ 0.f, m_data.spokeThickness / 2.f });
      spoke.setPosition({0.f, 0.f}); // centered around particle
      spoke.setRotation( sf::radians( angle ) );
      spoke.setFillColor( m_data.fillStartColor );
    }
  }

  void updateDonutRing()
  {
    m_donutRing.clear();
    constexpr int segments = 64;
    constexpr float angleStep = NX_TAU / static_cast<float>(segments);

    for (int32_t i = 0; i <= segments; ++i)
    {
      const float angle = i * angleStep;
      const sf::Vector2f dir({std::cos(angle), std::sin(angle)});

      const sf::Vector2f outerPoint = dir * m_radiusOverride;
      const sf::Vector2f innerPoint = dir * m_data.innerRadius;

      m_donutRing.append(sf::Vertex(outerPoint, m_data.outlineStartColor));
      m_donutRing.append(sf::Vertex(innerPoint, m_data.outlineStartColor));
    }
  }


private:
  const BurstRingParticleData_t& m_data;

  float m_radiusOverride { 0.f };

  sf::VertexArray m_donutRing { sf::PrimitiveType::TriangleStrip };

  sf::CircleShape m_ring;
  std::vector< sf::RectangleShape > m_spokes;
};
}