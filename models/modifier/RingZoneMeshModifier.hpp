#pragma once

namespace nx
{
  /// this works really well with Fractal layouts
  class RingZoneMeshModifier final : public IParticleModifier
  {
    struct RingZoneMeshData_t
    {
      bool isActive = true;
      float ringSpacing = 100.f;
      bool drawRings = true;
      bool drawSpokes = true;
      sf::Color lineColor = sf::Color(255, 255, 255, 100);

      float pulseSpeed = 1.0f; // Hz
      float minAlpha = 32.f;
      float maxAlpha = 200.f;
      bool enablePulse = true;
    };

  public:
    explicit RingZoneMeshModifier(const GlobalInfo_t &info) : m_globalInfo(info) {}

    E_ModifierType getType() const override { return E_ModifierType::E_RingZoneMeshModifier; }

    nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( getType() ) },
        { "isActive", m_data.isActive },
        { "ringSpacing", m_data.ringSpacing },
        { "drawRings", m_data.drawRings },
        { "drawSpokes", m_data.drawSpokes },
        { "lineColor", SerialHelper::convertColorToJson( m_data.lineColor ) },
        { "pulseSpeed", m_data.pulseSpeed },
        { "minAlpha", m_data.minAlpha },
         { "maxAlpha", m_data.maxAlpha },
         { "enablePulse", m_data.enablePulse }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        m_data.isActive = j[ "isActive" ].get<bool>();
        m_data.ringSpacing = j.at( "ringSpacing" ).get<float>();
        m_data.drawRings = j.at( "drawRings" ).get<bool>();
        m_data.pulseSpeed = j.at( "pulseSpeed" ).get<float>();
        m_data.minAlpha = j.at( "minAlpha" ).get<float>();
        m_data.maxAlpha = j.at( "maxAlpha" ).get<float>();
        m_data.enablePulse = j.at( "enablePulse" ).get<bool>();
        m_data.lineColor = SerialHelper::convertColorFromJson( j.at( "lineColor" ) );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    void drawMenu() override
    {
      if (ImGui::TreeNode("Test Modifier"))
      {
        ImGui::SliderFloat("Ring Spacing", &m_data.ringSpacing, 20.f, 200.f);
        ImGui::Checkbox("Draw Ring Loops", &m_data.drawRings);
        ImGui::Checkbox("Draw Radials", &m_data.drawSpokes);
        ImVec4 color = m_data.lineColor;
        if (ImGui::ColorEdit4("Line Color", reinterpret_cast< float * >(&color)))
          m_data.lineColor = color;

        ImGui::SeparatorText("Pulse Alpha");
        ImGui::Checkbox("Enable Pulse", &m_data.enablePulse);
        if (m_data.enablePulse)
        {
          ImGui::SliderFloat("Pulse Speed (Hz)", &m_data.pulseSpeed, 0.1f, 10.0f);
          ImGui::SliderFloat("Min Alpha", &m_data.minAlpha, 0.f, 255.f);
          ImGui::SliderFloat("Max Alpha", &m_data.maxAlpha, 0.f, 255.f);
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update(const sf::Time &) override {}

    bool isActive() const override { return m_data.isActive; }

    void processMidiEvent(const Midi_t &) override {}

    void modify(const ParticleLayoutData_t &, std::deque< TimedParticle_t * > &particles,
                std::deque< sf::Drawable * > &outArtifacts) override
    {
      float alpha = m_data.lineColor.a; // default static alpha

      if (m_data.enablePulse)
      {
        float time = m_globalInfo.elapsedTimeSeconds;
        float t = std::sin(time * m_data.pulseSpeed * NX_TAU); // [-1, 1]
        float normalized = 0.5f * (t + 1.f); // [0, 1]
        alpha = m_data.minAlpha + (m_data.maxAlpha - m_data.minAlpha) * normalized;
      }

      sf::Color pulsedColor = m_data.lineColor;
      pulsedColor.a = static_cast< uint8_t >(alpha);

      auto *lines = new sf::VertexArray(sf::PrimitiveType::Lines);
      const sf::Vector2f &center = m_globalInfo.windowHalfSize;

      // Step 1: Group particles into rings
      std::map< int, std::vector< TimedParticle_t * > > rings;
      for (auto *p: particles)
      {
        float dist = length(p->shape.getPosition() - center);
        int ringIdx = static_cast< int >(dist / m_data.ringSpacing);
        rings[ ringIdx ].push_back(p);
      }

      // Step 2: Draw rings and spokes
      // TimedParticle_t *prevInRing = nullptr;

      for (auto &[ ringIdx, ringParticles ]: rings)
      {
        if (ringParticles.size() < 2)
          continue;

        // Sort particles clockwise by angle
        std::ranges::sort(
        ringParticles, [ & ](TimedParticle_t *a, TimedParticle_t *b)
        { return angleFromCenter(center, a->shape.getPosition()) < angleFromCenter(center, b->shape.getPosition()); });

        if (m_data.drawRings)
        {
          for (size_t i = 0; i < ringParticles.size(); ++i)
          {
            auto *p1 = ringParticles[ i ];
            auto *p2 = ringParticles[ (i + 1) % ringParticles.size() ]; // wrap around
            // lines->append(sf::Vertex(p1->shape.getPosition(), m_lineColor));
            // lines->append(sf::Vertex(p2->shape.getPosition(), m_lineColor));

            lines->append(sf::Vertex(p1->shape.getPosition(), pulsedColor));
            lines->append(sf::Vertex(p2->shape.getPosition(), pulsedColor));
          }
        }

        if (m_data.drawSpokes && ringIdx > 0)
        {
          auto &prevRing = rings[ ringIdx - 1 ];
          size_t minCount = std::min(ringParticles.size(), prevRing.size());

          for (size_t i = 0; i < minCount; ++i)
          {
            lines->append(sf::Vertex(ringParticles[ i ]->shape.getPosition(), pulsedColor));
            lines->append(sf::Vertex(prevRing[ i ]->shape.getPosition(), pulsedColor));
          }
        }
      }

      outArtifacts.push_back(lines);
    }

  private:
    float length(const sf::Vector2f &v) const { return std::sqrt(v.x * v.x + v.y * v.y); }

    float angleFromCenter(const sf::Vector2f &center, const sf::Vector2f &pos) const
    {
      sf::Vector2f d = pos - center;
      return std::atan2(d.y, d.x);
    }

  private:
    const GlobalInfo_t &m_globalInfo;
    RingZoneMeshData_t m_data;

  };

} // namespace nx
