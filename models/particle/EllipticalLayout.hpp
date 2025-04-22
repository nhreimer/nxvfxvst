#pragma once

namespace nx
{

  struct EllipticalLayoutData_t : public ParticleLayoutData_t
  {
    float radiusX = 300.f;
    float radiusY = 200.f;
    float arcSpreadDegrees = 360.f; // full ring by default
    float rotationDegrees = 0.f;
    sf::Vector2f centerOffset = {0.f, 0.f};
    bool sequential = false;
  };

  class EllipticalLayout final : public ParticleLayoutBase< EllipticalLayoutData_t >
  {
  public:

    explicit EllipticalLayout( const GlobalInfo_t& globalInfo )
      : ParticleLayoutBase( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_EllipticalLayout; }

    void addMidiEvent(const Midi_t &midiEvent) override
    {
      const float arcRad = sf::degrees(m_data.arcSpreadDegrees).asRadians();
      const float baseAngle = arcRad * m_angleCursor;

      const float rotRad = sf::degrees(m_data.rotationDegrees).asRadians();
      const float angle = baseAngle + rotRad;

      const float x = std::cos(angle) * m_data.radiusX;
      const float y = std::sin(angle) * m_data.radiusY;

      const sf::Vector2f pos = m_globalInfo.windowHalfSize + m_data.centerOffset + sf::Vector2f(x, y);

      auto * p = m_particles.emplace_back( new TimedParticle_t() );
      p->shape.setPosition(pos);

      ParticleLayoutBase::initializeParticle( p, midiEvent );

      // advance cursor
      m_angleCursor += 1.f / 12.f; // 12 evenly spaced notes per full ring
      if (m_angleCursor > 1.f) m_angleCursor -= 1.f;
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Elliptical Layout" ) )
      {
        ImGui::Checkbox( "Sequential", &m_data.sequential );
        ImGui::SliderFloat("Radius X", &m_data.radiusX, 50.f, 1000.f);
        ImGui::SliderFloat("Radius Y", &m_data.radiusY, 50.f, 1000.f);
        ImGui::SliderFloat("Arc Spread (deg)", &m_data.arcSpreadDegrees, 10.f, 360.f);
        ImGui::SliderFloat("Ellipse Rotation (deg)", &m_data.rotationDegrees, -180.f, 180.f);
        ImGui::SliderFloat2("Center Offset", &m_data.centerOffset.x, -500.f, 500.f);

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    float m_angleCursor = 0.f; // keeps track of where to place next particle
  };

}