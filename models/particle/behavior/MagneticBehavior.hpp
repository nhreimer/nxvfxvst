#pragma once

namespace nx
{

  class MagneticAttractorBehavior final : public IParticleBehavior
  {

    struct MagneticData_t
    {
      bool isAttracting = true;
      float strength = 100.f;
      bool useFalloff = true;
      float falloffExponent = 1.5f;
      //bool followMouse = false;
      sf::Vector2f magnetLocation { 0.5f, 0.5f };
    };

  public:
    explicit MagneticAttractorBehavior(const GlobalInfo_t &info) : m_globalInfo(info) {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }
    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_BehaviorType getType() const override { return E_MagneticBehavior; }

    void applyOnSpawn(TimedParticle_t *, const Midi_t&) override
    {}

    void applyOnUpdate(TimedParticle_t * p, const sf::Time& dt) override
    {
      const sf::Vector2f pos = p->shape.getPosition();

      const sf::Vector2f attractor { m_data.magnetLocation.x * static_cast< float >(m_globalInfo.windowSize.x ),
                                      m_data.magnetLocation.y * static_cast< float >(m_globalInfo.windowSize.y) };

      const sf::Vector2f dir = attractor - pos;
      const float distance = std::max(length(dir), 0.001f); // avoid divide by 0
      const sf::Vector2f normDir = dir / distance;

      // Falloff based on distance
      float force = m_data.strength * 2.f;
      if (m_data.useFalloff)
        force *= 1.f / std::pow(distance, m_data.falloffExponent);

      // Direction: pull or push
      if (!m_data.isAttracting)
        force *= -1.f;

      // Apply movement offset
      const sf::Vector2f offset = normDir * force * dt.asSeconds();
      p->shape.move( offset );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Magnetic Behavior" ) )
      {
        ImGui::Checkbox("Attracting", &m_data.isAttracting);
        ImGui::SliderFloat("Strength", &m_data.strength, 0.0f, 500.0f);
        ImGui::Checkbox("Use Falloff", &m_data.useFalloff);
        ImGui::SliderFloat("Falloff Exponent", &m_data.falloffExponent, 0.1f, 4.0f);
        // ImGui::Checkbox("Follow Mouse", &m_followMouse);

        if ( ImGui::SliderFloat( "Magnet x##1", &m_data.magnetLocation.x, 0.f, 1.f ) ||
             ImGui::SliderFloat( "Magnet y##1", &m_data.magnetLocation.y, 0.f, 1.f ) )
        {
          const sf::Vector2f calibrated { m_data.magnetLocation.x * static_cast< float >( m_globalInfo.windowSize.x ),
                                          m_data.magnetLocation.y * static_cast< float >( m_globalInfo.windowSize.y ) };
          m_timedCursor.setPosition( calibrated );
        }
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

  private:

    static float length(const sf::Vector2f& v)
    {
      return std::sqrt(v.x * v.x + v.y * v.y);
    }

  private:
    const GlobalInfo_t& m_globalInfo;

    MagneticData_t m_data;
    TimedCursorPosition m_timedCursor;
  };


}