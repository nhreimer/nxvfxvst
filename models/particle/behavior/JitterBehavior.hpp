#pragma once

#include "models/IParticleBehavior.hpp"

namespace nx
{

  class JitterBehavior : public IParticleBehavior
  {
    struct JitterData_t
    {
      float jitterMultiplier { 0.5 };
    };

  public:
    explicit JitterBehavior( const GlobalInfo_t& info )
      : m_globalInfo( info )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
   {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "jitterMultiplier", m_data.jitterMultiplier }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      m_data.jitterMultiplier = j.at( "jitterMultiplier" ).get<float>();
    }

    E_BehaviorType getType() const override { return E_JitterBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override
    {
      p->shape.setPosition( getJitterPosition( p ) );
    }

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override
    {
      p->shape.setPosition( getJitterPosition( p ) );
    }

  public:
    void drawMenu() override
    {
      ImGui::SliderFloat( "Jitter", &m_data.jitterMultiplier, 0.f, 5.f );
    }

  private:
    sf::Vector2f getJitterPosition( const TimedParticle_t * p )
    {
      // add jitter to the position
      // 1. get the circular offset by calculating a random angle [0 - 360)
      const auto jitterAngle = static_cast< float >( m_rand() % 360 ) * NX_D2R;

      // 2. get the deviation amount
      // we have to add 1.f here to prevent the radius from ever being 0 and having a div/0 error.
      auto safeRadius = static_cast< uint32_t >( p->shape.getRadius() );
      if ( safeRadius == 0 ) ++safeRadius;

      const auto jitterAmount = m_data.jitterMultiplier *
                                     static_cast< float >( m_rand() % safeRadius );
      return p->shape.getPosition() +
        sf::Vector2f { std::cos( jitterAngle ) * jitterAmount, std::sin( jitterAngle ) * jitterAmount };
    }

  private:
      const GlobalInfo_t& m_globalInfo;

      std::mt19937 m_rand;
      JitterData_t m_data;

  };

}