#pragma once

namespace nx
{
  class CurvedLine final : public sf::Drawable
  {
  public:
    CurvedLine(const sf::Vector2f &start,
            const sf::Vector2f &end,
            const float curvature = 0.25f,
            const int segments = 32);

    void setWidth(const float width);

    void setEndpoints(const sf::Vector2f &start, const sf::Vector2f &end);

    void setCurvature(const float curvature);

    void setColor(const sf::Color color);

    void setGradient(const sf::Color& startColor, const sf::Color& endColor);

    void update();

  private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override { target.draw(m_vertices, states); }

    sf::Vector2f m_start;
    sf::Vector2f m_end;
    sf::Color m_colorStart;
    sf::Color m_colorEnd;
    float m_curvature;
    int m_segments;
    float m_width { 1.f };
    sf::Color m_color;

    sf::VertexArray m_vertices;
  };

} // namespace nx
