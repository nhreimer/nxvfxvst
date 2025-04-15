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

    E_ModifierType getType() const override { return E_ModifierType::E_TestModifier; }

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

    }

  private:

  private:
    const GlobalInfo_t& m_globalInfo;

    bool m_enabled = true;

  };

} // namespace nx
