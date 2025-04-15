#pragma once

namespace nx
{

  struct LissajousCurveLayoutData_t : public ParticleLayoutData_t
  {
    float phaseAStep { 2.0f };
    float phaseBStep { 3.0f };

    float phaseDelta { 0.5f };
    float phaseSpread { 0.5f };


    int32_t tracerCount = 8;
    float tracerDelayStep = 0.04f; // how far apart each ghost is in phase
    float fadeExponent = 1.5f;     // higher = faster fade
    float sizeFalloff = 0.75f;     // scaling down radius
  };

  /// This layout places each particle along a mathematically beautiful Lissajous curve using parametric sine functions
  /// x = A * sin(a * t + δ)
  /// y = B * sin(b * t)
  /// a = pitch-based frequency
  /// b = velocity-based frequency
  /// t = time or phase (based on global time)
  /// δ = offset to prevent overlap or to animate drift
  class LissajousCurveLayout final : public ParticleConsumer< LissajousCurveLayoutData_t >
  {
  public:

    explicit LissajousCurveLayout( const GlobalInfo_t& globalInfo )
      : ParticleConsumer( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );

      j[ "phaseAStep" ] = m_data.phaseAStep;
      j[ "phaseBStep" ] = m_data.phaseBStep;
      j[ "phaseDelta" ] = m_data.phaseDelta;
      j[ "phaseSpread" ] = m_data.phaseSpread;
      j[ "tracerCount" ] = m_data.tracerCount;
      j[ "tracerDelayStep" ] = m_data.tracerDelayStep;
      j[ "fadeExponent" ] = m_data.fadeExponent;
      j[ "sizeFalloff" ] = m_data.sizeFalloff;

      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      ParticleHelper::deserialize( m_data, j );
      m_data.phaseAStep = j.value( "phaseAStep", 2.0f );
      m_data.phaseBStep = j.value( "phaseBStep", 3.0f );
      m_data.phaseDelta = j.value( "phaseDelta", 0.5f );
      m_data.phaseSpread = j.value( "phaseSpread", 0.5f );
      m_data.tracerCount = j.value( "tracerCount", 8 );
      m_data.tracerDelayStep = j.value( "tracerDelayStep", 0.5f );
      m_data.fadeExponent = j.value( "fadeExponent", 1.0f );
      m_data.sizeFalloff = j.value( "sizeFalloff", 0.5f );
    }

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LissajousCurveLayout; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Lissajous Curve Layout" ) )
      {
        ParticleHelper::drawMenu( m_data );

        //ImGui::SliderFloat("Phase", &m_data.phase, 0.f, 1.0f);
        ImGui::SliderFloat("a Phase Step", &m_data.phaseAStep, 0.f, 5.0f);
        ImGui::SliderFloat("b Phase Step", &m_data.phaseBStep, 0.f, 5.0f);
        ImGui::SliderFloat("Phase Delta", &m_data.phaseDelta, 0.f, 2.0f);
        ImGui::SliderFloat("Phase Spread", &m_data.phaseSpread, 0.f, 1.0f);

        ImGui::Separator();

        ImGui::SliderInt( "Tracer Count", &m_data.tracerCount, 0, 128 );
        ImGui::SliderFloat( "Tracer Delay Step", &m_data.tracerDelayStep, 0.0f, 3.0f );
        ImGui::SliderFloat( "Fade Exponent", &m_data.fadeExponent, 0.0f, 1.0f );
        ImGui::SliderFloat( "Size Falloff", &m_data.sizeFalloff, 0.0f, 1.0f );

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Separator();
      }
    }

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      ParticleConsumer::addMidiEvent( midiEvent );

      const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
      const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency

      const auto * lastAdded = m_particles.back();

      for ( int i = 0; i <= m_data.tracerCount; ++i )
      {
        const float localT = m_globalInfo.elapsedTimeSeconds - i * m_data.tracerDelayStep;

        const float x = ( m_globalInfo.windowSize.x * m_data.phaseSpread ) * sin(a * localT + m_data.phaseDelta);
        const float y = ( m_globalInfo.windowSize.y * m_data.phaseSpread ) * sin(b * localT);

        auto * p = m_particles.emplace_back( new TimedParticle_t( *lastAdded ) );
        const float lifeScale = 1.0f - std::pow(static_cast<float>(i) / m_data.tracerCount, m_data.fadeExponent);
        const float sizeScale = std::pow(m_data.sizeFalloff, static_cast<float>(i));

        p->shape.setRadius( 5.f * sizeScale );
        p->shape.setPosition( { x, y } );

        auto faded = p->shape.getFillColor();
        faded.a = static_cast< uint8_t >( 255 * lifeScale ); // fade out
        p->shape.setFillColor(faded);
        p->initialColor = faded;

        p->spawnTime = m_globalInfo.elapsedTimeSeconds;
        p->timeLeft = static_cast<int32_t>(m_data.timeoutInMS * lifeScale); // shorter life
      }
    }

  protected:
    sf::Vector2f getNextPosition( const Midi_t & midiEvent ) override
    {
      const float a = m_data.phaseAStep + static_cast< float >(midiEvent.pitch % 4); // X frequency
      const float b = m_data.phaseBStep + static_cast< float >(static_cast< int32_t >(midiEvent.velocity) % 5); // Y frequency

      // this is also the phase
      const float t = m_globalInfo.elapsedTimeSeconds;

      const float x = ( m_globalInfo.windowSize.x * m_data.phaseSpread ) * sin(a * t + m_data.phaseDelta);
      const float y = ( m_globalInfo.windowSize.y * m_data.phaseSpread ) * sin(b * t);

      return { x, y };
    }

  private:

  };
}