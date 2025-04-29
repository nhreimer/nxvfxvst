#pragma once

#include <random>

#include "helpers/MathHelper.hpp"
#include "helpers/MidiHelper.hpp"

namespace nx
{

  class SpiralParticleLayout final : public ParticleLayoutBase< ParticleLayoutData_t >
  {
  public:

    explicit SpiralParticleLayout( PipelineContext& context )
      : ParticleLayoutBase( context )
    {}

    ~SpiralParticleLayout() override = default;

    [[nodiscard]]
    E_LayoutType getType() const override { return E_LayoutType::E_SpiralLayout; }

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

    void addMidiEvent(const Midi_t &midiEvent) override
    {
      auto * p = m_particles.emplace_back( new TimedParticle_t() );
      p->shape.setPosition( getNextPosition( midiEvent ) );
      ParticleLayoutBase::initializeParticle( p, midiEvent );
    }

    void drawMenu() override
    {
      ImGui::Text( "Particles: %d", m_particles.size() );
      ImGui::Separator();
      if ( ImGui::TreeNode( "Spiral Layout " ) )
      {
        ParticleHelper::drawMenu( m_data );
        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  protected:


    sf::Vector2f getNextPosition( const Midi_t& midiNote ) const
    {
      const auto noteInfo = MidiHelper::getMidiNote( midiNote.pitch );

      const auto noteNumber = std::get< 0 >( noteInfo );
      const auto noteOctave = std::get< 1 >( noteInfo );

      const auto position = MathHelper::getAnglePosition( 12,
                                                      noteNumber,
                                                      static_cast< float >( noteOctave ),
                                                      static_cast< float >( noteOctave ) );

      return { m_ctx.globalInfo.windowHalfSize.x + position.x,
                  m_ctx.globalInfo.windowHalfSize.y + position.y };
    }

  private:

  };

}