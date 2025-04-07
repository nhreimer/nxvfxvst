#pragma once

namespace nx
{
  class PassthroughParticleModifier final : public IParticleModifier
  {
  public:

    explicit PassthroughParticleModifier( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {}

    ~PassthroughParticleModifier() override = default;

    nlohmann::json serialize() const override
    {
      return
      {
        { "type", getType() }
      };
    }

    void deserialize( const nlohmann::json& j ) override {}

    E_ModifierType getType() const override { return E_NoModifier; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Passthrough" ) )
      {
        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override {}

    sf::RenderTexture & modifyParticles( const ParticleLayoutData_t& particleLayoutData, std::deque< TimedParticle_t >& particles ) override
    {
      if ( m_outputTexture.getSize() != m_winfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_winfo.windowSize ) )
        {
          LOG_ERROR( "failed to resize texture" );
        }
      }
      m_outputTexture.clear();

      for ( const auto& particle : particles )
        m_outputTexture.draw( particle.shape, particleLayoutData.blendMode );

      m_outputTexture.display();
      return m_outputTexture;
    }

  private:
    const GlobalInfo_t& m_winfo;
    sf::RenderTexture m_outputTexture;

  };
}