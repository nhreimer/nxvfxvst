#pragma once

#include "models/particle/ParticleLayoutBase.hpp"

namespace nx
{

class RandomParticleLayout final : public ParticleLayoutBase< ParticleLayoutData_t >
{
public:
  [[nodiscard]]
  nlohmann::json serialize() const override
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    return j;
  }

  void deserialize(const nlohmann::json &j) override
  {
    ParticleHelper::deserialize( m_data, j );
    if (j.contains("behaviors"))
      m_behaviorPipeline.loadPipeline(j.at("behaviors"));
  }

  explicit RandomParticleLayout( PipelineContext& context )
  : ParticleLayoutBase( context )
  {}

  E_LayoutType getType() const override { return E_LayoutType::E_RandomLayout; }

  void drawMenu() override
  {
    ImGui::Text( "Particles: %d", m_particles.size() );
    ImGui::Separator();
    if ( ImGui::TreeNode( "Random Layout " ) )
    {
      ParticleHelper::drawMenu( m_data );

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void addMidiEvent(const Midi_t &midiEvent) override
  {
    auto * p = m_particles.emplace_back( new TimedParticle_t() );
    p->shape.setPosition( getNextPosition( midiEvent ) );
    ParticleLayoutBase::initializeParticle( p, midiEvent );
  }

protected:
  sf::Vector2f getNextPosition( const Midi_t & midiNote )
  {
    return { static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.x ),
                  static_cast< float >( m_rand() % m_ctx.globalInfo.windowSize.y ) };
  }

private:

  std::mt19937 m_rand;

};

}