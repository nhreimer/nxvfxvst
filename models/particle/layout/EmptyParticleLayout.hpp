#pragma once

namespace nx
{

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class EmptyParticleLayout final : public IParticleLayout
  {
  public:

    explicit EmptyParticleLayout( PipelineContext& )
    {}

    [[nodiscard]] nlohmann::json serialize() const override
    {
      return
      {
        { "type", SerialHelper::serializeEnum( getType() ) }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {}

    [[nodiscard]] E_LayoutType getType() const override { return E_LayoutType::E_EmptyLayout; }
    void addMidiEvent(const Midi_t &midiEvent) override {}
    void update(const sf::Time &deltaTime) override {}
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Empty Layout" ) )
      {
        ImGui::Text( "No Options Available" );
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t &getParticleLayoutData() const override
    {
      return m_layoutData;
    }

    [[nodiscard]]
    const ParticleData_t &getParticleData() const override
    {
      return m_particleData;
    }

    [[nodiscard]]
    std::deque< IParticle * > &getParticles() override
    {
      return m_particles;
    }

  private:

    ParticleData_t m_particleData;
    ParticleLayoutData_t m_layoutData;
    std::deque< IParticle * > m_particles;
  };

}