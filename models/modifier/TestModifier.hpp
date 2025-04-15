#pragma once

namespace nx
{
  /// this is a class fleshed out for testing purposes only
  class TestModifier final : public IParticleModifier
  {
  public:
    explicit TestModifier( const GlobalInfo_t& info )
      : m_globalInfo( info )
    {}

    E_ModifierType getType() const override { return E_TestModifier; }

    nlohmann::json serialize() const override
    {
      return
      {
      };
    }

    void deserialize( const nlohmann::json& j ) override
    {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Test Modifier" ) )
      {
        ImGui::SliderFloat("Ring Spacing", &m_ringSpacing, 20.f, 200.f);
        ImGui::Checkbox("Draw Ring Loops", &m_drawRings);
        ImGui::Checkbox("Draw Radials", &m_drawSpokes);
        ImVec4 color = m_lineColor;
        if ( ImGui::ColorEdit4("Line Color", reinterpret_cast<float*>(&color)) )
          m_lineColor = color;

        ImGui::SeparatorText("Pulse Alpha");
        ImGui::Checkbox("Enable Pulse", &m_enablePulse);
        if (m_enablePulse) {
          ImGui::SliderFloat("Pulse Speed (Hz)", &m_pulseSpeed, 0.1f, 10.0f);
          ImGui::SliderFloat("Min Alpha", &m_minAlpha, 0.f, 255.f);
          ImGui::SliderFloat("Max Alpha", &m_maxAlpha, 0.f, 255.f);
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update(const sf::Time &) override
    {
    }

    bool isActive() const override { return m_enabled; }

    void processMidiEvent(const Midi_t &) override
    {
    }

    void modify(const ParticleLayoutData_t &, std::deque< TimedParticle_t * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override
    {
      float alpha = m_lineColor.a; // default static alpha

      if (m_enablePulse) {
        float time = m_globalInfo.elapsedTimeSeconds;
        float t = std::sin(time * m_pulseSpeed * 2.f * 3.14159265f); // [-1, 1]
        float normalized = 0.5f * (t + 1.f);                         // [0, 1]
        alpha = m_minAlpha + (m_maxAlpha - m_minAlpha) * normalized;
      }

      sf::Color pulsedColor = m_lineColor;
      pulsedColor.a = static_cast<uint8_t>(alpha);

      auto* lines = new sf::VertexArray(sf::PrimitiveType::Lines);
      const sf::Vector2f& center = m_globalInfo.windowHalfSize;

      // Step 1: Group particles into rings
      std::map<int, std::vector<TimedParticle_t*>> rings;
      for (auto* p : particles) {
        float dist = length(p->shape.getPosition() - center);
        int ringIdx = static_cast<int>(dist / m_ringSpacing);
        rings[ringIdx].push_back(p);
      }

      // Step 2: Draw rings and spokes
      TimedParticle_t* prevInRing = nullptr;

      for (auto& [ringIdx, ringParticles] : rings) {
        if (ringParticles.size() < 2) continue;

        // Sort particles clockwise by angle
        std::sort(ringParticles.begin(), ringParticles.end(),
          [&](TimedParticle_t* a, TimedParticle_t* b) {
            return angleFromCenter(center, a->shape.getPosition()) <
                   angleFromCenter(center, b->shape.getPosition());
          });

        if (m_drawRings) {
          for (size_t i = 0; i < ringParticles.size(); ++i) {
            auto* p1 = ringParticles[i];
            auto* p2 = ringParticles[(i + 1) % ringParticles.size()]; // wrap around
            // lines->append(sf::Vertex(p1->shape.getPosition(), m_lineColor));
            // lines->append(sf::Vertex(p2->shape.getPosition(), m_lineColor));
            lines->append(sf::Vertex(p1->shape.getPosition(), pulsedColor));
            lines->append(sf::Vertex(p2->shape.getPosition(), pulsedColor));

          }
        }

        if (m_drawSpokes && ringIdx > 0) {
          auto& prevRing = rings[ringIdx - 1];
          size_t minCount = std::min(ringParticles.size(), prevRing.size());

          for (size_t i = 0; i < minCount; ++i) {
            lines->append(sf::Vertex(ringParticles[i]->shape.getPosition(), pulsedColor));
            lines->append(sf::Vertex(prevRing[i]->shape.getPosition(), pulsedColor));
          }
        }
      }

      outArtifacts.push_back(lines);
    }

  private:

  float length(const sf::Vector2f& v) const {
    return std::sqrt(v.x * v.x + v.y * v.y);
  }

  float angleFromCenter(const sf::Vector2f& center, const sf::Vector2f& pos) const {
    sf::Vector2f d = pos - center;
    return std::atan2(d.y, d.x);
  }
  private:
    const GlobalInfo_t& m_globalInfo;

    float m_ringSpacing = 100.f;
    bool m_drawRings = true;
    bool m_drawSpokes = true;
    sf::Color m_lineColor = sf::Color(255, 255, 255, 100);

    float m_pulseSpeed = 1.0f;           // Hz
    float m_minAlpha = 32.f;
    float m_maxAlpha = 200.f;
    bool  m_enablePulse = true;


    bool m_enabled = true;

  };

} // namespace nx
