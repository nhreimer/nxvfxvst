#pragma once

namespace nx
{

  class GradientLine final : public sf::Drawable
  {
  public:

    GradientLine() = default;

    GradientLine( const sf::Vector2f &start,
          const sf::Vector2f &end,
          const float width,
          const sf::Color colorStart = sf::Color::White,
          const sf::Color colorEnd = sf::Color::White )
      : m_start( start ),
        m_end( end ),
        m_width( width ),
        m_colorStart( colorStart ),
        m_colorEnd( colorEnd )
    {
      updateVertices();
    }

    void setStart( const sf::Vector2f &newStart )
    {
      m_start = newStart;
      updateVertices();
    }

    void setEnd( const sf::Vector2f &newEnd )
    {
      m_end = newEnd;
      updateVertices();
    }

    void setWidth( const float newWidth )
    {
      m_width = newWidth;
      updateVertices();
    }

    void setGradient( const sf::Color newColorStart,
                      const sf::Color newColorEnd )
    {
      m_colorStart = newColorStart;
      m_colorEnd = newColorEnd;
      updateVertices();
    }

    void setColorStart( const sf::Color newColorStart )
    {
      m_colorStart = newColorStart;
      updateVertices();
    }

    void setColorEnd( const sf::Color newColorEnd )
    {
      m_colorEnd = newColorEnd;
      updateVertices();
    }

  private:

    void updateVertices()
    {
      const auto direction = m_end - m_start;
      const auto sqrd = std::sqrt( direction.x * direction.x + direction.y * direction.y );
      const auto unitDir = ( sqrd == 0.f ) ? direction : direction / sqrd;
      const sf::Vector2f normal(-unitDir.y, unitDir.x);
      const sf::Vector2f offset = normal * ( m_width / 2.0f );

      m_vertices = sf::VertexArray( sf::PrimitiveType::TriangleStrip, 4 );

      m_vertices[ 0 ].position = m_start - offset;
      m_vertices[ 0 ].color = m_colorStart;

      m_vertices[ 1 ].position = m_start + offset;
      m_vertices[ 1 ].color = m_colorStart;

      m_vertices[ 2 ].position = m_end - offset;
      m_vertices[ 2 ].color = m_colorEnd;

      m_vertices[ 3 ].position = m_end + offset;
      m_vertices[ 3 ].color = m_colorEnd;
    }

    void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
    {
      target.draw( m_vertices, states );
    }

  private:
    sf::Vector2f m_start;
    sf::Vector2f m_end;
    float m_width { 0.f };
    sf::Color m_colorStart;
    sf::Color m_colorEnd;
    sf::VertexArray m_vertices;
  };

}