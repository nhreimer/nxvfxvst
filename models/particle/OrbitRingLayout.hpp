#pragma once

#include "helpers/CommonHeaders.hpp"

namespace nx
{

struct OrbitRingParticleLayoutData_t : public ParticleLayoutData_t
{
  float orbitRadius { 0.f };
  float angleOffset { 1.5f };
  float orbitSpeed { 1.f };
};

struct OrbitRingParticle_t : public TimedParticle_t
{
  float orbitRadius { 80.f };
  float angleOffset { 1.5f };
};

class OrbitRingLayout final : public IParticleLayout
{

public:

  explicit OrbitRingLayout( const GlobalInfo_t& globalInfo )
    : m_globalInfo( globalInfo )
  {}

  nlohmann::json serialize() const override
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "orbitRadius" ] = m_data.orbitRadius;
    j[ "angleOffset" ] = m_data.angleOffset;
    j[ "orbitSpeed" ] = m_data.orbitSpeed;
    return j;
  }

  void deserialize(const nlohmann::json &j) override
  {
    ParticleHelper::deserialize( m_data, j );
    m_data.orbitRadius = j.value( "orbitRadius", 10.f );
    m_data.angleOffset = j.value( "angleOffset", 1.5f );
    m_data.orbitSpeed = j.value( "orbitSpeed", 1.f );
  }

  E_LayoutType getType() const override { return E_OrbitRingLayout; };

  void drawMenu() override
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Particle Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );
      ImGui::Separator();

      if ( ImGui::TreeNode( "Orbit Ring Options" ) )
      {
        ImGui::SliderFloat( "Orbit Radius##1", &m_data.orbitRadius, 0.f, 200.f );
        ImGui::SliderFloat( "Orbit Speed##1", &m_data.orbitSpeed, 0.f, 10.f );

        ImGui::TreePop();
        ImGui::Separator();
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void addMidiEvent(const Midi_t &midiEvent) override
  {
    const auto noteInfo = MidiHelper::getMidiNote( midiEvent.pitch );
    // auto position = getNextPosition( noteInfo );
    // position += { static_cast< float >( m_globalInfo.windowSize.x ) / 2.f,
    //               static_cast< float >( m_globalInfo.windowSize.y ) / 2.f };

    auto * timeParticle = m_particles.emplace_back( new TimedParticle_t() );
    auto& particle = timeParticle->shape;

    particle.setRadius( m_data.radius +
                        m_data.velocitySizeMultiplier * midiEvent.velocity );

    timeParticle->initialColor = ColorHelper::getColorPercentage(
      m_data.startColor,
      std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

    // particle.setPosition( position );
    particle.setPointCount( m_data.shapeSides );
    particle.setFillColor( timeParticle->initialColor );
    particle.setOutlineThickness( m_data.outlineThickness );
    particle.setOutlineColor( m_data.outlineColor );

    particle.setOrigin( particle.getGlobalBounds().size / 2.f );

    // timestamp it
    timeParticle->spawnTime = m_clock.getElapsedTime().asSeconds();
  }

  void update( const sf::Time &deltaTime ) override
  {
    const sf::Vector2f center = { m_globalInfo.windowSize.x / 2.f, m_globalInfo.windowSize.y / 2.f };
    const float timeNow = m_clock.getElapsedTime().asSeconds();

    for ( auto i = 0; i < m_particles.size(); ++i )
    {
      auto * particle = m_particles[ i ];

      // ORBIT ADJUSTMENTS
      const auto * orbitParticle = static_cast< OrbitRingParticle_t * >( particle );
      const float elapsed = timeNow - orbitParticle->spawnTime;
      const float angle = orbitParticle->angleOffset + elapsed * m_data.orbitSpeed;
      const sf::Vector2f pos = center + sf::Vector2f( cos( angle ), sin( angle ) ) * m_data.orbitRadius;
      particle->shape.setPosition( pos );

      // NORMAL FADE-OUT ADJUSTMENTS
      const auto& timeParticle = m_particles[ i ];
      timeParticle->timeLeft += deltaTime.asMilliseconds();
      const auto percentage = static_cast< float >( timeParticle->timeLeft ) /
                         static_cast< float >( m_data.timeoutInMS );

      if ( percentage < 1.f )
      {
        const auto nextColor =
          ColorHelper::getNextColor(
            timeParticle->initialColor,
            m_data.endColor,
            percentage );

        timeParticle->shape.setFillColor( nextColor );
      }
      else
      {
        delete particle;
        m_particles.erase( m_particles.begin() + i );
      }
    }
  }

  [[nodiscard]]
  const ParticleLayoutData_t &getParticleOptions() const override { return m_data; }

  [[nodiscard]]
  std::deque< TimedParticle_t * > &getParticles() override { return m_particles; }

private:

  const GlobalInfo_t& m_globalInfo;

  std::mt19937 m_rand;

  std::deque< TimedParticle_t* > m_particles;
  OrbitRingParticleLayoutData_t m_data;
  sf::Clock m_clock;
};

}