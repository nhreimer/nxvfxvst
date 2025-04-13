#pragma once

#include "models/IParticleModifier.hpp"

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

    bool isActive() const override { return m_isActive; }
    void processMidiEvent(const Midi_t &midiEvent) override {}

    [[nodiscard]]
    E_ModifierType getType() const override { return E_FreeFallModifier; }
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Free Fall" ) )
      {
        ImGui::SliderFloat( "##Time Divisor", &m_data.timeDivisor, 0.5f, 50.f, "Time Divisor %0.2f" );
        MenuHelper::drawBlendOptions( m_data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override
    {
      m_accumulator += deltaTime;
    }

    void modify(
       const ParticleLayoutData_t& particleLayoutData,
       std::deque< TimedParticle_t* >& particles,
       std::deque< sf::Drawable* >& outArtifacts ) override
    {
      // this adjusts particles, it doesn't add any new artifacts
      // which may mess with upstream modifiers, so be careful of the ordering
      // whenever using free fall
      for ( int i = 0; i < particles.size(); ++i )
      {
        auto& shape = particles[ i ]->shape;

        const auto trail = particles[ i ]->spawnTime / m_data.timeDivisor;
        shape.setPosition( { shape.getPosition().x, shape.getPosition().y + trail } );
      }
    }

  private:

    const GlobalInfo_t& m_globalInfo;

    FreeFallData_t m_data;

    sf::Time m_accumulator;
    bool m_isActive { true };

  };


}