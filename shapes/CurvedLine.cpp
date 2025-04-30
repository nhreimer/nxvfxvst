#include "shapes/CurvedLine.hpp"

namespace nx
{

    CurvedLine::CurvedLine(const sf::Vector2f &start,
            const sf::Vector2f &end,
            const float curvature,
            const int segments)
        : m_start(start),
          m_end(end),
          m_curvature(curvature),
          m_segments(segments)
    {
      // m_vertices.setPrimitiveType(sf::PrimitiveType::LineStrip);
      // m_vertices.resize(m_segments + 1);
      m_vertices.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
      m_vertices.resize((m_segments + 1) * 2);
      update();
    }

    void CurvedLine::setWidth(const float width)
    {
      m_width = width;
      update();
    }

    void CurvedLine::setEndpoints(const sf::Vector2f &start, const sf::Vector2f &end)
    {
      m_start = start;
      m_end = end;
      update();
    }

    void CurvedLine::setCurvature(const float curvature)
    {
      m_curvature = curvature;
      update();
    }

    void CurvedLine::setColor(const sf::Color color)
    {
      m_color = color;
      update();
    }

    void CurvedLine::setGradient(const sf::Color& startColor, const sf::Color& endColor)
    {
      m_colorStart = startColor;
      m_colorEnd = endColor;
      update();
    }

    void CurvedLine::update()
    {
      const auto dir = m_end - m_start;
      const auto mid = 0.5f * (m_start + m_end);
      sf::Vector2f normal(-dir.y, dir.x);
      const float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
      if (len != 0.f) normal /= len;

      const auto control = mid + normal * m_curvature * std::sqrt(dir.x * dir.x + dir.y * dir.y);

      for (int i = 0; i <= m_segments; ++i)
      {
        const float t = static_cast<float>(i) / m_segments;
        const float u = 1.f - t;

        const auto point = u * u * m_start + 2.f * u * t * control + t * t * m_end;

        // Tangent for direction
        sf::Vector2f tangent =
            2.f * (1.f - t) * (control - m_start) +
            2.f * t * (m_end - control);

        sf::Vector2f normal(-tangent.y, tangent.x);
        float nLen = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (nLen != 0.f)
          normal /= nLen;

        const auto offset = normal * (m_width * 0.5f);

        // Interpolated color
        const auto color = sf::Color(
            static_cast<uint8_t>(m_colorStart.r + t * (m_colorEnd.r - m_colorStart.r)),
            static_cast<uint8_t>(m_colorStart.g + t * (m_colorEnd.g - m_colorStart.g)),
            static_cast<uint8_t>(m_colorStart.b + t * (m_colorEnd.b - m_colorStart.b)),
            static_cast<uint8_t>(m_colorStart.a + t * (m_colorEnd.a - m_colorStart.a))
        );

        m_vertices[i * 2 + 0] = sf::Vertex(point - offset, color);
        m_vertices[i * 2 + 1] = sf::Vertex(point + offset, color);

        // m_vertices[i * 2 + 0] = sf::Vertex(point - offset, m_color);
        // m_vertices[i * 2 + 1] = sf::Vertex(point + offset, m_color);
      }


      // for (int i = 0; i <= m_segments; ++i)
      // {
      //   const float t = static_cast< float >(i) / m_segments;
      //   const float u = 1.f - t;
      //   const auto point = u * u * m_start + 2.f * u * t * control + t * t * m_end;
      //   m_vertices[ i ] = sf::Vertex(point, m_color);
      // }
    }

} // namespace nx
