#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "helpers/ColorHelper.hpp"
#include "log/Logger.hpp"

#include "models/IParticle.hpp"
#include "models/data/ParticleData_t.hpp"

#include "models/particle/particles/TimedParticleBase.hpp"

namespace nx
{

  class CircleParticle final : public TimedParticleBase
  {
  public:

    CircleParticle() = delete;

    CircleParticle( const ParticleData_t& data,
                    const float spawnTimeStampInSeconds )
      : m_data( data ),
        m_radiusOverride( m_data.radius )
    {
      m_spawnTimeInSeconds = spawnTimeStampInSeconds;
      update();
    }

    CircleParticle( const ParticleData_t& data,
                    const float spawnTimeStampInSeconds,
                    const float radiusOverride )
      : m_data( data ),
        m_radiusOverride( radiusOverride )
    {
      m_spawnTimeInSeconds = spawnTimeStampInSeconds;
      update();
    }

    CircleParticle(const CircleParticle &other) = delete;
    CircleParticle & operator=( const CircleParticle& other ) = delete;
    CircleParticle( CircleParticle&& other ) = delete;
    CircleParticle& operator=( CircleParticle&& other ) = delete;

    IParticle * clone( const float timeStampInSeconds ) const override
    {
      auto * p = new CircleParticle( m_data, timeStampInSeconds, m_radiusOverride );
      p->m_spawnTimeInSeconds = timeStampInSeconds;
      return p;
    }

    uint8_t getPointCount() const override { return m_data.pointCount; }

    float getOutlineThickness() const override { return m_data.outlineThickness; }

    sf::FloatRect getLocalBounds() const override { return m_bounds; }

    sf::FloatRect getGlobalBounds() const override
    {
      return getTransform().transformRect(getLocalBounds());
    }

    float getRadius() const override { return m_data.radius; }

    void setColorPattern(  const sf::Color & startColor, const sf::Color & endColor ) override
    {
      updateVertexColors( m_vertices, startColor, endColor );
    }

    [[nodiscard]]
    std::pair< sf::Color, sf::Color > getColors() const override
    {
      return std::make_pair( m_data.fillStartColor, m_data.fillEndColor );
    }

    [[nodiscard]]
    std::pair< sf::Color, sf::Color > getOutlineColors() const override
    {
      return std::make_pair( m_data.outlineStartColor, m_data.outlineEndColor );
    }

    void setOutlineColorPattern( const sf::Color & startColor, const sf::Color & endColor ) override
    {
      updateVertexColors( m_outlineVertices, startColor, endColor );
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
      states.transform *= getTransform();
      states.coordinateType = sf::CoordinateType::Pixels;

      // Render the inside
      target.draw(m_vertices, states);

      // Render the outline
      if (m_data.outlineThickness != 0)
      {
        states.texture = nullptr;
        target.draw(m_outlineVertices, states);
      }
    }

  private:

    sf::Vector2f getPointPosition(const uint8_t index) const
    {
      const auto angle =
      static_cast< float >(index) / static_cast< float >(m_data.pointCount) * sf::degrees(360.f) - sf::degrees(90.f);

      return sf::Vector2f(m_radiusOverride, m_radiusOverride) +
             sf::Vector2f(m_radiusOverride, angle);
    }

    void update()
    {
      const auto count = getPointCount();
      if (count < 3)
      {
        m_vertices.clear();
        m_outlineVertices.clear();
        return;
      }

      // + 2 for the center and repeated first point
      m_vertices.resize(count + 2);

      // Position
      for (auto i = 0; i < count; ++i)
        m_vertices[ i + 1 ].position = getPointPosition(i);

      m_vertices[ count + 1 ].position = m_vertices[ 1 ].position;

      // Update the bounding rectangle
      m_vertices[ 0 ] = m_vertices[ 1 ]; // so that the result of getBounds() is correct
      m_insideBounds = m_vertices.getBounds();

      // Compute the center and make it the first vertex
      m_vertices[ 0 ].position = m_insideBounds.getCenter();

      // Outline
      updateOutline();
    }

    void updateOutline()
    {
      // Return if there is no outline
      if (m_data.outlineThickness == 0.f)
      {
        m_outlineVertices.clear();
        m_bounds = m_insideBounds;
        return;
      }

      const std::size_t count = m_vertices.getVertexCount() - 2;
      m_outlineVertices.resize((count + 1) * 2);

      for (std::size_t i = 0; i < count; ++i)
      {
        const std::size_t index = i + 1;

        // Get the two segments shared by the current point
        const auto p0 = (i == 0) ? m_vertices[ count ].position : m_vertices[ index - 1 ].position;
        const auto p1 = m_vertices[ index ].position;
        const auto p2 = m_vertices[ index + 1 ].position;

        // Compute their normal
        auto n1 = computeNormal(p0, p1);
        auto n2 = computeNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (n1.dot(m_vertices[ 0 ].position - p1) > 0)
          n1 = -n1;
        if (n2.dot(m_vertices[ 0 ].position - p1) > 0)
          n2 = -n2;

        // Combine them to get the extrusion direction
        const auto factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const auto normal = (n1 + n2) / factor;

        // Update the outline points
        m_outlineVertices[ i * 2 + 0 ].position = p1;
        m_outlineVertices[ i * 2 + 1 ].position = p1 + normal * m_data.outlineThickness;
      }

      // Duplicate the first point at the end, to close the outline
      m_outlineVertices[ count * 2 + 0 ].position = m_outlineVertices[ 0 ].position;
      m_outlineVertices[ count * 2 + 1 ].position = m_outlineVertices[ 1 ].position;

      // Update the shape's bounds
      m_bounds = m_outlineVertices.getBounds();
    }

    static void updateVertexColors( sf::VertexArray & vertexArray,
                                    const sf::Color & startColor,
                                    const sf::Color & endColor )
    {

      const auto n = static_cast< int32_t >( vertexArray.getVertexCount() );
      const auto nf = static_cast< float >( vertexArray.getVertexCount() );
      for ( int32_t i = 0, y = n - 1; i < n / 2; ++i, --y )
      {
        const float percentage = static_cast< float >( i + 1 ) / nf;
        const auto currentColor = ColorHelper::lerpColor( startColor, endColor, percentage );
        vertexArray[ i ].color = currentColor;
        vertexArray[ y ].color = currentColor;
      }

      if ( n % 2 == 1 )
      {
        const int32_t mid = ( n / 2 - 1 );
        const float percentage  = static_cast< float >( mid ) / nf;
        vertexArray[ mid ].color = ColorHelper::lerpColor( startColor, endColor, percentage );
      }
    }

    // Compute the normal of a segment
    static sf::Vector2f computeNormal(const sf::Vector2f p1, const sf::Vector2f p2)
    {
      auto normal = (p2 - p1).perpendicular();
      const float length = normal.length();
      if (length != 0.f)
        normal /= length;
      return normal;
    }

  private:

    const ParticleData_t& m_data;
    const float m_radiusOverride { 0.f };

    sf::VertexArray m_vertices        { sf::PrimitiveType::TriangleFan   };
    sf::VertexArray m_outlineVertices { sf::PrimitiveType::TriangleStrip };

    sf::FloatRect m_insideBounds;
    sf::FloatRect m_bounds;
  };
} // namespace nx
