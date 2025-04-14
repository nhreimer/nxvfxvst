#pragma once

namespace nx
{

struct FractalRingLayoutData_t : public ParticleLayoutData_t
{
  int32_t depthLimit { 2 };
  int32_t baseRingCount { 4 };
  float radiusAdjustment { 0.75f };
  float radialSpread { 1.f };
  float delayFractalFadesMultiplier { 1.25f };
};

///
/// This is good for single notes because it hits the center, like a kick drum or something
class FractalRingLayout final : public IParticleLayout
{
public:

  explicit FractalRingLayout( const GlobalInfo_t& globalInfo )
    : m_globalInfo( globalInfo )
  {}

  [[nodiscard]]
  nlohmann::json serialize() const override
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "depthLimit" ] = m_data.depthLimit;
    j[ "radialSpread" ] = m_data.radialSpread;
    j[ "baseRingCount" ] = m_data.baseRingCount;
    j[ "radiusAdjustment" ] = m_data.radiusAdjustment;
    j[ "delayFractalFadesMultiplier" ] = m_data.delayFractalFadesMultiplier;
    return j;
  }

  void deserialize(const nlohmann::json &j) override
  {
    ParticleHelper::deserialize( m_data, j );
    m_data.depthLimit = j["depthLimit"];
    m_data.radialSpread = j["radialSpread"];
    m_data.baseRingCount = j["baseRingCount"];
    m_data.radiusAdjustment = j["radiusAdjustment"];
    m_data.delayFractalFadesMultiplier = j["delayFractalFadesMultiplier"];
  }

  [[nodiscard]]
  E_LayoutType getType() const override { return E_FractalRingLayout; }

  void addMidiEvent( const Midi_t &midiEvent ) override
  {
    // const float angle = ( ( midiEvent.pitch / 127.f ) * NX_TAU );

    const sf::Vector2f pos =
    {
      // m_globalInfo.windowHalfSize.x + std::cos( angle ) * m_data.radius,
      // m_globalInfo.windowHalfSize.y + std::sin( angle ) * m_data.radius
      m_globalInfo.windowHalfSize.x,
      m_globalInfo.windowHalfSize.y
    };

    auto * p = createParticle( midiEvent, pos, m_data.radius );
    p->shape.setPosition( pos );
    spawnFractalRing( midiEvent, m_data.depthLimit, m_data.radius, pos );
  }

  void update( const sf::Time &deltaTime ) override
  {
    for ( auto i = 0; i < m_particles.size(); ++i )
    {
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
        delete m_particles[ i ];
        m_particles.erase( m_particles.begin() + i );
      }
    }
  }

  void drawMenu() override
  {
    if ( ImGui::TreeNode( "Fractal Ring Layout" ) )
    {
      ParticleHelper::drawMenu( m_data );
      ImGui::Separator();
      ImGui::SliderInt("Spawn Depth", &m_data.depthLimit, 0, 4);
      ImGui::SliderInt("Base Ring Count", &m_data.baseRingCount, 0, 8);
      ImGui::SliderFloat( "Radius Adjustment", &m_data.radiusAdjustment, 0.f, 1.f );
      ImGui::SliderFloat( "Radial Spread", &m_data.radialSpread, 0.f, 5.f );
      ImGui::SliderFloat( "Fractal Depth Fade Offset", &m_data.delayFractalFadesMultiplier, 0.f, 5.f );

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  [[nodiscard]]
  const ParticleLayoutData_t & getParticleOptions() const override { return m_data; }

  [[nodiscard]]
  std::deque< TimedParticle_t * > & getParticles() override { return m_particles; }

private:

  void spawnFractalRing( const Midi_t& midiEvent,
                         const int depth,
                         const float adjustedRadius,
                         const sf::Vector2f& lastPosition )
  {
    // base case
    if ( depth <= 0 ) return;

    const float lastRadius = adjustedRadius / m_data.radiusAdjustment;

    const float angleStep = NX_TAU / m_data.baseRingCount;

    for (int i = 0; i < m_data.baseRingCount; ++i)
    {
      const float angle = i * angleStep;

      sf::Vector2f pos =
      {
        lastPosition.x + std::cos(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread ),
        lastPosition.y + std::sin(angle) * ( ( adjustedRadius + lastRadius ) * m_data.radialSpread )
      };

      auto* p = createParticle(midiEvent, pos, adjustedRadius);
      p->shape.setPosition(pos);
      p->timeLeft -= ( m_data.delayFractalFadesMultiplier * depth * m_data.timeoutInMS );

      spawnFractalRing(
        midiEvent,
        depth - 1,
        adjustedRadius * m_data.radiusAdjustment,
        pos
      );
    }

  }

  TimedParticle_t* createParticle( const Midi_t& midiEvent,
                                   const sf::Vector2f& position,
                                   const float adjustedRadius )
  {
    auto * p = m_particles.emplace_back( new TimedParticle_t() );
    p->spawnTime = m_globalInfo.elapsedTimeSeconds;

    p->initialColor = ColorHelper::getColorPercentage(
        m_data.startColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

    auto& shape = p->shape;
    shape.setPosition( position );
    shape.setRadius( adjustedRadius );
    shape.setFillColor( p->initialColor );

    shape.setOutlineThickness( m_data.outlineThickness );
    shape.setOutlineColor( m_data.outlineColor );
    shape.setOrigin( shape.getGlobalBounds().size / 2.f );

    return p;
  }

private:
  const GlobalInfo_t& m_globalInfo;
  std::deque< TimedParticle_t * > m_particles;
  FractalRingLayoutData_t m_data;
};

}