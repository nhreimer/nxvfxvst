#include "models/particle/layout/EllipticalLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json EllipticalLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "radiusX" ] = m_data.radiusX;
    j[ "radiusY" ] = m_data.radiusY;
    j[ "arcSpreadDegrees" ] = m_data.arcSpreadDegrees;
    j[ "rotationDegrees" ] = m_data.rotationDegrees;
    j[ "sequential" ] = m_data.sequential;
    j[ "slices" ] = m_data.slices;
    j[ "centerOffset" ] = { { "x", m_data.centerOffset.x }, { "y", m_data.centerOffset.y } };

    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGeneratorManager.getParticleGenerator()->serialize();
    j[ "easings" ] = m_fadeEasing.serialize();
    return j;
  }

  void EllipticalLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.radiusX = j.value( "radiusX", 300.f );
      m_data.radiusY = j.value( "radiusY", 200.f );
      m_data.arcSpreadDegrees = j.value( "arcSpreadDegrees", 360.f );
      m_data.rotationDegrees = j.value( "rotationDegrees", 0.f );
      m_data.sequential = j.value( "sequential", true );
      m_data.slices = j.value( "slices", 12.f );

      m_data.centerOffset = sf::Vector2f { j[ "centerOffset" ].value( "x", 0.f ),
                                             j[ "centerOffset" ].value( "y", 0.f ) };
    }

    if ( j.contains( "particleGenerator" ) )
      m_particleGeneratorManager.getParticleGenerator()->deserialize( j.at( "particleGenerator" ) );

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j.at( "behaviors" ) );

    if ( j.contains( "easings" ) )
      m_fadeEasing.deserialize( j.at( "easings" ) );
  }

  void EllipticalLayout::addMidiEvent(const Midi_t &midiEvent)
  {
    if ( m_data.sequential ) addSequentialParticle( midiEvent );
    else addParticle( midiEvent );
  }

  void EllipticalLayout::drawMenu()
  {
    if ( ImGui::TreeNode( "Elliptical Layout" ) )
    {
      m_particleGeneratorManager.drawMenu();
      ImGui::Separator();
      m_particleGeneratorManager.getParticleGenerator()->drawMenu();

      ImGui::SeparatorText( "Elliptical Options" );

      ImGui::Checkbox( "Sequential", &m_data.sequential );
      ImGui::SliderFloat("Radius X", &m_data.radiusX, 50.f, 1000.f);
      ImGui::SliderFloat("Radius Y", &m_data.radiusY, 50.f, 1000.f);
      ImGui::SliderFloat("Arc Spread (deg)", &m_data.arcSpreadDegrees, 10.f, 360.f);
      ImGui::SliderFloat("Ellipse Rotation (deg)", &m_data.rotationDegrees, -180.f, 180.f);
      ImGui::SliderFloat2("Center Offset", &m_data.centerOffset.x, -500.f, 500.f);
      ImGui::SliderFloat("Slices", &m_data.slices, 1.f, 36.f);

      ImGui::SeparatorText( "Behaviors" );
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void EllipticalLayout::addSequentialParticle( const Midi_t& midiEvent )
  {
    const float arcRad = sf::degrees(m_data.arcSpreadDegrees).asRadians();
    const float baseAngle = arcRad * m_angleCursor;

    const float rotRad = sf::degrees(m_data.rotationDegrees).asRadians();
    const float angle = baseAngle + rotRad;

    const float x = std::cos(angle) * m_data.radiusX;
    const float y = std::sin(angle) * m_data.radiusY;

    const sf::Vector2f pos = m_ctx.globalInfo.windowHalfSize + m_data.centerOffset + sf::Vector2f(x, y);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( pos );

    ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );

    // advance cursor
    m_angleCursor += 1.f / m_data.slices; // 12 evenly spaced notes per full ring
    if (m_angleCursor > 1.f) m_angleCursor -= 1.f;
  }

  void EllipticalLayout::addParticle( const Midi_t& midiEvent )
  {
    const float angleSlice = midiEvent.pitch * 1.f / m_data.slices;
    const float arcRad = sf::degrees(m_data.arcSpreadDegrees).asRadians();
    const float baseAngle = arcRad * angleSlice;

    const float rotRad = sf::degrees(m_data.rotationDegrees).asRadians();
    const float angle = baseAngle + rotRad;

    const float x = std::cos(angle) * m_data.radiusX;
    const float y = std::sin(angle) * m_data.radiusY;

    const sf::Vector2f pos = m_ctx.globalInfo.windowHalfSize + m_data.centerOffset + sf::Vector2f(x, y);

    auto * p = m_particles.emplace_back(
      m_particleGeneratorManager.getParticleGenerator()->createParticle(
        midiEvent, m_ctx.globalInfo.elapsedTimeSeconds ) );

    p->setPosition( pos );

    ParticleLayoutBase::notifyBehaviorOnSpawn( p, midiEvent );
  }

}