#pragma once

#include "helpers/ColorHelper.hpp"
#include "helpers/SerialHelper.hpp"

#include "models/data/ParticleLayoutData_t.hpp"

namespace nx
{

  /// Provides midi consumption, i.e., adding it to the deque
  /// Provides updates based on timeouts (can be overridden)
  /// must provide setPosition
  class ParticleConsumer : public IParticleLayout
  {
    public:

    explicit ParticleConsumer( const GlobalInfo_t& winfo )
      : m_globalInfo( winfo )
    {}

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json serialize() const override
    {
      return
      {
           { "type", getType() },
        { "startColor", SerialHelper::convertColorToJson(m_data.startColor) },
        { "endColor", SerialHelper::convertColorToJson(m_data.endColor) },
        { "outlineColor", SerialHelper::convertColorToJson(m_data.outlineColor) },
        { "outlineThickness", m_data.outlineThickness },
        { "radius", m_data.radius },
        { "shapeSides", m_data.shapeSides },
        { "timeoutInMS", m_data.timeoutInMS },
        { "spreadMultiplier", m_data.spreadMultiplier },
        { "jitterMultiplier", m_data.jitterMultiplier },
        { "positionOffset", SerialHelper::convertVectorToJson(m_data.positionOffset) },
        { "boostVelocity", m_data.boostVelocity },
        { "velocitySizeMultiplier", m_data.velocitySizeMultiplier },
        { "blendMode", SerialHelper::convertBlendModeToString( m_data.blendMode ) }
      };
    }

    void deserialize(const nlohmann::json& j) override
    {
      m_data.startColor = SerialHelper::convertColorFromJson(j.at("startColor"), sf::Color::White);
      m_data.endColor = SerialHelper::convertColorFromJson(j.at("endColor"), sf::Color::Black);
      m_data.outlineColor = SerialHelper::convertColorFromJson(j.at("outlineColor"), sf::Color::White);
      m_data.outlineThickness = j.value("outlineThickness", 0.f);
      m_data.radius = j.value("radius", 30.f);
      m_data.shapeSides = j.value("shapeSides", 30);
      m_data.timeoutInMS = j.value("timeoutInMS", 1500);
      m_data.spreadMultiplier = j.value("spreadMultiplier", 1.f);
      m_data.jitterMultiplier = j.value("jitterMultiplier", 0.f);
      m_data.positionOffset = SerialHelper::convertVectorFromJson< float >(j.at("positionOffset"));
      m_data.boostVelocity = j.value("boostVelocity", 0.f);
      m_data.velocitySizeMultiplier = j.value("velocitySizeMultiplier", 0.f);
      m_data.blendMode = SerialHelper::convertBlendModeFromString(j.value("blendMode", "None"));
    }

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      const auto noteInfo = MidiHelper::getMidiNote( midiEvent.pitch );
      auto position = getNextPosition( noteInfo );
      position += { static_cast< float >( m_globalInfo.windowSize.x ) / 2.f,
                    static_cast< float >( m_globalInfo.windowSize.y ) / 2.f };

      auto& timeParticle = m_particles.emplace_back();
      auto& particle = timeParticle.shape;

      particle.setRadius( m_data.radius +
                          m_data.velocitySizeMultiplier * midiEvent.velocity );

      timeParticle.initialColor = ColorHelper::getColorPercentage(
        m_data.startColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      particle.setPosition( position );
      particle.setPointCount( m_data.shapeSides );
      particle.setFillColor( timeParticle.initialColor );
      particle.setOutlineThickness( m_data.outlineThickness );
      particle.setOutlineColor( m_data.outlineColor );

      particle.setOrigin( particle.getGlobalBounds().size / 2.f );
    }

    void update( const sf::Time &deltaTime ) override
    {
      for ( auto i = 0; i < m_particles.size(); ++i )
      {
        auto& timeParticle = m_particles[ i ];
        timeParticle.timeLeft += deltaTime.asMilliseconds();
        const auto percentage = static_cast< float >( timeParticle.timeLeft ) /
                           static_cast< float >( m_data.timeoutInMS );

        if ( percentage < 1.f )
        {
          const auto nextColor =
            ColorHelper::getNextColor(
              timeParticle.initialColor,
              m_data.endColor,
              percentage );

          timeParticle.shape.setFillColor( nextColor );
        }
        else
        {
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleOptions() const override { return m_data; }

    [[nodiscard]]
    std::deque< TimedParticle_t > &getParticles() override { return m_particles; }

    protected:

    virtual sf::Vector2f getNextPosition( const std::tuple< int32_t, int32_t >& noteInfo ) = 0;

    protected:

      const GlobalInfo_t& m_globalInfo;
      std::deque< TimedParticle_t > m_particles;

      std::mt19937 m_rand;
      ParticleLayoutData_t m_data;

  };

}