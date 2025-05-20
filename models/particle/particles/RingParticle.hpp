#pragma once

#include <SFML/Graphics/VertexArray.hpp>

#include "models/particle/particles/TimedParticleBase.hpp"
#include "models/data/ParticleData_t.hpp"

#include "helpers/MathHelper.hpp"

namespace nx
{

  struct RingParticleData_t : public ParticleData_t
  {
    float width { 10.f };
  };

  class RingParticle final : public TimedParticleBase
  {

  public:

    RingParticle( const RingParticleData_t& data,
                  const float timeStamp )
      : m_data( data ),
        m_radiusOverride( data.radius ),
        m_ring( sf::PrimitiveType::TriangleStrip, data.pointCount * 2 + 2 )
    {
      m_spawnTimeInSeconds = timeStamp;
      updateShape();
      setColorPattern( m_data.fillStartColor, m_data.fillEndColor );
    }

    RingParticle( const RingParticleData_t& data,
                  const float spawnTimeStampInSeconds,
                  const float radiusOverride )
      : m_data( data ),
        m_radiusOverride( radiusOverride )
    {
      m_spawnTimeInSeconds = spawnTimeStampInSeconds;
      updateShape();
      setColorPattern( m_data.fillStartColor, m_data.fillEndColor );
    }

    RingParticle(const RingParticle &other) = delete;
    RingParticle & operator=( const RingParticle& other ) = delete;
    RingParticle( RingParticle&& other ) = delete;
    RingParticle& operator=( RingParticle&& other ) = delete;

    // clones the particle and gives it the associated timestamp
    IParticle * clone( const float timeStampInSeconds ) const override
    {
      auto * p = new RingParticle( m_data, timeStampInSeconds, m_radiusOverride );
      p->m_spawnTimeInSeconds = m_spawnTimeInSeconds;
      return p;
    }

    uint8_t getPointCount() const override { return m_data.pointCount; }

    float getOutlineThickness() const override { return m_data.outlineThickness; }

    float getRadius() const override { return m_radiusOverride; }

    sf::FloatRect getLocalBounds() const override
    {
      return m_ring.getBounds();
    }

    sf::FloatRect getGlobalBounds() const override
    {
      return getTransform().transformRect( m_ring.getBounds() );
    }

    void setColorPattern( const sf::Color & startColor, const sf::Color & endColor ) override
    {
      const auto n = static_cast< int32_t >(m_ring.getVertexCount());
      const auto nf = static_cast< float >( m_ring.getVertexCount() );
      for ( int32_t i = 0, y = n - 1; i < n / 2; ++i, --y )
      {
        const float percentage = static_cast< float >(i + 1) / nf;
        const auto currentColor = ColorHelper::lerpColor( startColor, endColor, percentage );
        m_ring[ i ].color = currentColor;
        m_ring[ y ].color = currentColor;
      }

      if ( n % 2 == 1 )
      {
        const int32_t mid = ( n / 2 - 1 );
        const float percentage  = static_cast< float >( mid ) / nf;
        m_ring[ mid ].color = ColorHelper::lerpColor( startColor, endColor, percentage );
      }
    }

    std::pair< sf::Color, sf::Color > getColors() const override
    {
      return std::make_pair( m_data.fillStartColor, m_data.fillEndColor );
    }

    void setOutlineColorPattern( const sf::Color & startColor, const sf::Color & endColor ) override
    {
      // TODO: add outlines
    }

    std::pair< sf::Color, sf::Color > getOutlineColors() const override
    {
      return std::make_pair( m_data.outlineStartColor, m_data.outlineEndColor );
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
      states.transform *= getTransform();
      target.draw(m_ring, states);
    }

  private:

    void updateShape()
    {
      // -2 because we have to get the final segment of the circle to connect
      // back to the starting 2
      const float angle = NX_TAU / ( static_cast< float >(m_ring.getVertexCount()) - 2 );

      for ( int i = 0; i < m_ring.getVertexCount(); i += 2 )
      {
        const float theta = static_cast< float >(i) * angle;
        const float x = std::cos( theta );
        const float y = std::sin( theta );

        // outer
        m_ring[ i ].position = { m_radiusOverride * x, m_radiusOverride * y };

        // inner
        m_ring[ i + 1 ].position = { ( m_radiusOverride - m_data.width ) * x,
                                     ( m_radiusOverride - m_data.width ) * y };
      }
    }

  private:

    const RingParticleData_t& m_data;
    float m_radiusOverride { 0.f };

    sf::VertexArray m_ring;
  };

}