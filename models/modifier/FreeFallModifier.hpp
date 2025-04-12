#pragma once

namespace nx
{

  struct FreeFallData_t
  {
    bool isActive { true };
    float timeDivisor { 2.5f };     // in seconds
    sf::BlendMode blendMode { sf::BlendNone };
  };

  class FreeFallModifier final : public IParticleModifier
  {
  public:

    explicit FreeFallModifier( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( E_FreeFallModifier ) },
          { "isActive", m_data.isActive },
          { "timeDivisor", m_data.timeDivisor },
          { "blendMode", SerialHelper::convertBlendModeToString( m_data.blendMode ) }
      };
    }
    void deserialize(const nlohmann::json &j) override
    {
      m_data.isActive = j.at( "isActive" ).get<bool>();
      m_data.timeDivisor = j.at( "timeDivisor" ).get<float>();
      m_data.blendMode = SerialHelper::convertBlendModeFromString( j.at( "blendMode" ).get<std::string>() );
    }

    [[nodiscard]]
    E_ModifierType getType() const override { return E_FreeFallModifier; }
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Trail Echoes" ) )
      {
        ImGui::SliderFloat( "##Time Divisor", &m_data.timeDivisor, 0.5f, 50.f, "Time Divisor %0.2f" );
        MenuHelper::drawBlendOptions( m_data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

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

        const auto trail = particles[ i ]->spawnTime / m_data.timeDivisor;
        shape.setPosition( { shape.getPosition().x, shape.getPosition().y + trail } );

        m_outputTexture.draw( shape, m_data.blendMode );
      }

      m_outputTexture.display();
      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    FreeFallData_t m_data;

    sf::Time m_accumulator;

    sf::RenderTexture m_outputTexture;

  };


}