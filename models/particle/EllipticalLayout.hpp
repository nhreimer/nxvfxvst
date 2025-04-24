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
    float slices = 12.f;
  };

  class EllipticalLayout final : public ParticleLayoutBase< EllipticalLayoutData_t >
  {
  public:

    explicit EllipticalLayout( const GlobalInfo_t& globalInfo )
      : ParticleLayoutBase( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
      j[ "behaviors" ] = m_behaviorPipeline.saveModifierPipeline();
      j[ "radiusX" ] = m_data.radiusX;
      j[ "radiusY" ] = m_data.radiusY;
      j[ "arcSpreadDegrees" ] = m_data.arcSpreadDegrees;
      j[ "rotationDegrees" ] = m_data.rotationDegrees;
      j[ "sequential" ] = m_data.sequential;
      j[ "slices" ] = m_data.slices;
      j[ "centerOffset" ] = SerialHelper::convertVec2ToJSON( m_data.centerOffset );
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        ParticleHelper::deserialize( m_data, j );
        m_data.radiusX = j.value( "radiusX", 300.f );
        m_data.radiusY = j.value( "radiusY", 200.f );
        m_data.arcSpreadDegrees = j.value( "arcSpreadDegrees", 360.f );
        m_data.rotationDegrees = j.value( "rotationDegrees", 0.f );
        m_data.sequential = j.value( "sequential", true );
        m_data.slices = j.value( "slices", 12.f );

        m_data.centerOffset = SerialHelper::convertVec2FromJSON< float >( j[ "centerOffset" ] );
      }
      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadModifierPipeline( j.at( "behaviors" ) );
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_EllipticalLayout; }

    void addMidiEvent(const Midi_t &midiEvent) override
    {
      if ( m_data.sequential ) addSequentialParticle( midiEvent );
      else addParticle( midiEvent );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Elliptical Layout" ) )
      {
        ParticleHelper::drawMenu(*reinterpret_cast< ParticleLayoutData_t * >(&m_data));

        ImGui::SeparatorText( "Elliptical Options" );

        ImGui::Checkbox( "Sequential", &m_data.sequential );
        ImGui::SliderFloat("Radius X", &m_data.radiusX, 50.f, 1000.f);
        ImGui::SliderFloat("Radius Y", &m_data.radiusY, 50.f, 1000.f);
        ImGui::SliderFloat("Arc Spread (deg)", &m_data.arcSpreadDegrees, 10.f, 360.f);
        ImGui::SliderFloat("Ellipse Rotation (deg)", &m_data.rotationDegrees, -180.f, 180.f);
        ImGui::SliderFloat2("Center Offset", &m_data.centerOffset.x, -500.f, 500.f);
        ImGui::SliderFloat("Slices", &m_data.slices, 1.f, 36.f);

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    void addSequentialParticle( const Midi_t& midiEvent )
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
      m_angleCursor += 1.f / m_data.slices; // 12 evenly spaced notes per full ring
      if (m_angleCursor > 1.f) m_angleCursor -= 1.f;
    }

    void addParticle( const Midi_t& midiEvent )
    {
      const float angleSlice = midiEvent.pitch * 1.f / m_data.slices;
      const float arcRad = sf::degrees(m_data.arcSpreadDegrees).asRadians();
      const float baseAngle = arcRad * angleSlice;

      const float rotRad = sf::degrees(m_data.rotationDegrees).asRadians();
      const float angle = baseAngle + rotRad;

      const float x = std::cos(angle) * m_data.radiusX;
      const float y = std::sin(angle) * m_data.radiusY;

      const sf::Vector2f pos = m_globalInfo.windowHalfSize + m_data.centerOffset + sf::Vector2f(x, y);

      auto * p = m_particles.emplace_back( new TimedParticle_t() );
      p->shape.setPosition(pos);

      ParticleLayoutBase::initializeParticle( p, midiEvent );
    }

  private:

    float m_angleCursor = 0.f; // keeps track of where to place next particle
  };

}