#pragma once

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

  struct TestParticleLayoutData_t : public ParticleLayoutData_t
  {
  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class TestParticleLayout final : public IParticleLayout
  {
  public:
    explicit TestParticleLayout(const GlobalInfo_t &globalInfo)
      : m_globalInfo( globalInfo ),
        m_behaviorPipeline( globalInfo )
    {}

    ~TestParticleLayout() override
    {
      for ( const auto * particle : m_particles )
        delete particle;
    }


    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return {};
    }

    void deserialize(const nlohmann::json &j) override {}

    E_LayoutType getType() const override { return E_TestLayout; }

    void drawMenu() override
    {
      ImGui::Text("Particles: %d", m_particles.size());
      ImGui::Separator();
      if (ImGui::TreeNode("Test Particle Layout"))
      {
        ParticleHelper::drawMenu(m_data);

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void addMidiEvent(const Midi_t &midiEvent) override
    {

    }

    void update( const sf::Time &deltaTime ) override
    {
      for ( auto i = 0; i < m_particles.size(); ++i )
      {
        const auto& timeParticle = m_particles[ i ];
        timeParticle->timeLeft += deltaTime.asMilliseconds();
        const auto percentage = static_cast< float >( timeParticle->timeLeft ) /
                           static_cast< float >( m_data.timeoutInMS );

        if ( percentage < 1.f )
        {
          const auto nextColor =
            ColorHelper::getNextColor(
              timeParticle->initialColor,
              m_data.endColor,
              percentage );

          timeParticle->shape.setFillColor( nextColor );
          m_behaviorPipeline.applyOnUpdate( timeParticle, deltaTime );
        }
        else
        {
          delete m_particles[ i ];
          m_particles.erase( m_particles.begin() + i );
        }
      }
    }

    [[nodiscard]]
    const ParticleLayoutData_t& getParticleOptions() const override { return m_data; }

    [[nodiscard]]
    std::deque< TimedParticle_t * > &getParticles() override { return m_particles; }

  private:


  private:
    const GlobalInfo_t & m_globalInfo;
    TestParticleLayoutData_t m_data;
    ParticleBehaviorPipeline m_behaviorPipeline;

    std::deque< TimedParticle_t* > m_particles;
  };

} // namespace nx
