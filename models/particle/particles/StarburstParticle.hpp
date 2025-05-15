#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <utility>

#include "helpers/MathHelper.hpp"
#include "models/particle/particles/TimedParticleBase.hpp"

//#include "utils/DebugCenter.hpp"

namespace nx
{

  struct StarburstParticleData_t : public ParticleData_t
  {
    // radius = outerRadius
    // innerRadius = the multiplier * outerRadius
    int32_t spikes { 5 };
    float innerRadiusMultiplier { 0.5f };
  };

class StarburstParticle final : public TimedParticleBase
{

public:

  StarburstParticle() = delete;

  StarburstParticle( const StarburstParticleData_t& data, const float timeStamp )
    : m_data( data ),
      m_radiusOverride( data.radius )
  {
    m_spawnTimeInSeconds = timeStamp;
    updateShape();
  }

  StarburstParticle( const StarburstParticleData_t& data,
                     const float timeStamp,
                     const float radius )
    : m_data( data )
  {
    m_spawnTimeInSeconds = timeStamp;
    m_radiusOverride = m_data.radius * m_data.innerRadiusMultiplier; // default multiplier
    updateShape();
  }

  IParticle * clone( const float timeStampInSeconds ) const override
  {
    auto * cloned = new StarburstParticle(m_data , timeStampInSeconds);
    cloned->setColorPattern(m_data.fillStartColor, m_data.fillEndColor);
    cloned->setOutlineColorPattern(m_data.outlineStartColor, m_data.outlineEndColor);
    return cloned;
  }

  uint8_t getPointCount() const override { return m_shape.getPointCount(); }
  float getOutlineThickness() const override { return m_shape.getOutlineThickness(); }
  float getRadius() const override { return m_radiusOverride; }

  sf::FloatRect getLocalBounds() const override { return m_shape.getLocalBounds(); }
  sf::FloatRect getGlobalBounds() const override { return getTransform().transformRect(m_shape.getGlobalBounds()); }

  void setColorPattern(const sf::Color& startColor, const sf::Color& endColor) override
  {
    m_shape.setFillColor(startColor);
  }

  std::pair<sf::Color, sf::Color> getColors() const override
  {
    return std::make_pair( m_data.fillStartColor, m_data.fillEndColor );
  }

  void setOutlineColorPattern(const sf::Color& startColor, const sf::Color& endColor) override
  {
    m_shape.setOutlineColor(startColor);
  }

  std::pair<sf::Color, sf::Color> getOutlineColors() const override
  {
    return std::make_pair( m_data.outlineStartColor, m_data.outlineEndColor );
  }

  void draw(sf::RenderTarget& target, sf::RenderStates states) const override
  {
    states.transform *= getTransform();
    target.draw(m_shape, states);
    // target.draw(m_shapeDebugCenter.debugCenter, states);
  }

private:
  void updateShape()
  {
    const int32_t pointCount = m_data.spikes * 2;
    m_shape.setPointCount(pointCount);

    const float angleStep = NX_TAU / pointCount;

    const float innerRadius = m_data.innerRadiusMultiplier * m_radiusOverride;

    for (int32_t i = 0; i < pointCount; ++i)
    {
      const float angle = i * angleStep - NX_PI / 2.f; // rotate to start at top
      const float radius = (i % 2 == 0) ? m_radiusOverride : innerRadius;
      const sf::Vector2f pt { std::cos(angle) * radius, std::sin(angle) * radius };
      m_shape.setPoint(i, pt);
    }

    m_shape.setOutlineThickness( m_data.outlineThickness );
    m_shape.setOutlineColor(m_data.outlineStartColor);
    m_shape.setFillColor(m_data.fillStartColor);
  }

private:

  const StarburstParticleData_t &m_data;
  float m_radiusOverride { 0.f };

  sf::ConvexShape m_shape;

  // DebugCenter m_shapeDebugCenter;

};

}