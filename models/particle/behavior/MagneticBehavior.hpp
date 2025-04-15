#pragma once

namespace nx
{

  class MagneticAttractorBehavior final : public IParticleBehavior
  {
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

      const sf::Vector2f attractor { m_magnetLocation.x * ( float )m_globalInfo.windowSize.x,
                                      m_magnetLocation.y * ( float )m_globalInfo.windowSize.y };

      sf::Vector2f dir = attractor - pos;
      float distance = std::max(length(dir), 0.001f); // avoid divide by 0
      sf::Vector2f normDir = dir / distance;

      // Falloff based on distance
      float force = m_strength * 2.f;
      if (m_useFalloff) {
        force *= 1.f / std::pow(distance, m_falloffExponent);
      }

      // Direction: pull or push
      if (!m_isAttracting)
        force *= -1.f;

      // Apply movement offset
      const sf::Vector2f offset = normDir * force * dt.asSeconds();
      p->shape.move( offset );
    }

    void drawMenu() override
    {
      ImGui::Checkbox("Attracting", &m_isAttracting);
      ImGui::SliderFloat("Strength", &m_strength, 0.0f, 500.0f);
      ImGui::Checkbox("Use Falloff", &m_useFalloff);
      ImGui::SliderFloat("Falloff Exponent", &m_falloffExponent, 0.1f, 4.0f);
      // ImGui::Checkbox("Follow Mouse", &m_followMouse);

      if ( ImGui::SliderFloat( "Magnet x##1", &m_magnetLocation.x, 0.f, 1.f ) ||
           ImGui::SliderFloat( "Magnet y##1", &m_magnetLocation.y, 0.f, 1.f ) )
      {
        const sf::Vector2f calibrated { m_magnetLocation.x * static_cast< float >( m_globalInfo.windowSize.x ),
                                        m_magnetLocation.y * static_cast< float >( m_globalInfo.windowSize.y ) };
        m_timedCursor.setPosition( calibrated );
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

    bool m_isAttracting = true;
    float m_strength = 100.f;
    bool m_useFalloff = true;
    float m_falloffExponent = 1.5f;
    bool m_followMouse = false;
    sf::Vector2f m_magnetLocation { 0.5f, 0.5f };

    TimedCursorPosition m_timedCursor;
  };


}