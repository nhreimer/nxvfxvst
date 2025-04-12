#pragma once

namespace nx
{

  class TrailEchoModifier final : public IParticleModifier
  {
  public:

    explicit TrailEchoModifier( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }
    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_ModifierType getType() const override { return E_TrailEchoModifier; }
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Trail Echoes" ) )
      {
        // ImVec4 color = m_trailColor;
        //
        // if ( ImGui::ColorPicker4( "Particle Fill##1",
        //                           reinterpret_cast< float * >( &color ),
        //                           ImGuiColorEditFlags_AlphaBar,
        //                           nullptr ) )
        // {
        //   m_trailColor = color;
        // }
        //
        // ImGui::SliderFloat("Trail Snapshot Interval##1", &m_snapshotInterval, 0.01f, 0.2f, "%.3f sec");
        // ImGui::SliderFloat("Trail Decay##1", &m_decayRate, 0.1f, 5.0f, "%.2f sec");
        // ImGui::SliderFloat("Trail Alpha##1", &m_trailAlpha, 10.f, 255.f);

        ImGui::Text( "Trails %d", m_trails );
        ImGui::SliderFloat( "##TrailAccumulator", &m_trailTimeAccumulation, 0.5f, 10.f, "Time Accumulation %0.2f" );
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override
    {
      m_deltaTime = deltaTime.asSeconds();
    }

    [[nodiscard]]
    sf::RenderTexture & modifyParticles( const ParticleLayoutData_t &particleLayoutData,
                                         std::deque< TimedParticle_t * > &particles ) override
    {
      if ( m_outputTexture.getSize() != m_globalInfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_globalInfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize sequential line texture" );
        }
      }

      m_outputTexture.clear( sf::Color::Transparent );

      for ( int i = 0; i < particles.size(); ++i )
      {
        auto& shape = particles[ i ]->shape;

        const auto trail = static_cast< int32_t >( ( particles[ i ]->spawnTime / m_trailTimeAccumulation ) );
        shape.setPosition( { shape.getPosition().x, shape.getPosition().y + trail } );

        m_outputTexture.draw( shape, m_blendMode );
      }

      m_outputTexture.display();
      return m_outputTexture;
    }

  private:

    struct GhostParticle
    {
      sf::Vector2f position;
      float lifetime { 1.0f };
    };

    const GlobalInfo_t& m_globalInfo;

    float m_deltaTime { 0.0f };

    float m_trailTimeAccumulation { 2.5f };

    std::vector< GhostParticle > m_trailParticles;
    sf::Time m_accumulator;

    bool m_isActive { true };

    sf::RenderTexture m_outputTexture;
    sf::BlendMode m_blendMode { sf::BlendNone };
    int32_t m_trails { 0 };

  };


}