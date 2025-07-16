#pragma once

#include "models/IParticle.hpp"
#include "models/IParticleGenerator.hpp"

#include "helpers/ColorHelper.hpp"

#include "models/data/ParticleData_t.hpp"
#include "models/data/Midi_t.hpp"

namespace nx
{

  template < typename TData >
  class ParticleGeneratorBase : public IParticleGenerator
  {
    static_assert( std::is_base_of_v< ParticleData_t, TData >,
                   "TData must derive from ParticleData_t" );

  public:

    ParticleGeneratorBase( PipelineContext& ctx,
                           const bool createVstBinding )
      : m_ctx( ctx )
    {
      // we put this here, so that child classes can perform their own
      // binding based on any modifications to ParticleData_t they've made
      if ( createVstBinding )
      {
        EXPAND_SHADER_VST_BINDINGS(PARTICLE_DATA_PARAMS, m_ctx.vstContext.paramBindingManager)
      }
    }

    ~ParticleGeneratorBase() override
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Particle Options" ) )
      {
        EXPAND_SHADER_IMGUI(PARTICLE_DATA_PARAMS, m_data)

        ImGui::TreePop();
        ImGui::Spacing();
      }
      // drawAppearanceMenu();
      // drawAdjustmentMenu();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      EXPAND_SHADER_PARAMS_TO_JSON(PARTICLE_DATA_PARAMS)
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(PARTICLE_DATA_PARAMS)
    }

    [[nodiscard]]
    const TData& getData() const override { return m_data; }

  protected:

    [[nodiscard]]
    TData& getData() { return m_data; }

    void initialize( IParticle * particle,
                     const Midi_t& midiEvent,
                     const float timeStampInSeconds )
    {
      initialize( particle, midiEvent.velocity, timeStampInSeconds );
    }

    void initialize( IParticle * particle,
                     const float velocity,
                     const float timeStampInSeconds )
    {
      const auto initialStartColor = ColorHelper::getColorPercentage(
        m_data.fillStartColor.first,
        std::min( velocity + m_data.boostVelocity.first, 1.f ) );

      const auto initialEndColor = ColorHelper::getColorPercentage(
        m_data.fillEndColor.first,
        std::min( velocity + m_data.boostVelocity.first, 1.f ) );

      particle->setColorPattern( initialStartColor, initialEndColor );
      // particle->setOutlineThickness( m_data.outlineThickness );

      const auto initialOutlineStartColor = ColorHelper::getColorPercentage(
        m_data.outlineStartColor.first,
        std::min( velocity + m_data.boostVelocity.first, 1.f ) );

      const auto initialOutlineEndColor = ColorHelper::getColorPercentage(
        m_data.outlineEndColor.first,
        std::min( velocity + m_data.boostVelocity.first, 1.f ) );

      particle->setOutlineColorPattern( initialOutlineStartColor, initialOutlineEndColor );

      particle->setColorPattern( m_data.fillStartColor.first, m_data.fillEndColor.first );

      particle->setExpirationTimeInSeconds( m_data.timeoutInSeconds.first + particle->getSpawnTimeInSeconds() );

      particle->setEnergy( velocity );
    }

  private:

    void drawAppearanceMenu()
    {
      if ( ImGui::TreeNode( "Particle Appearance" ) )
      {

        // ColorHelper::drawImGuiColorEdit4( "Particle Color A##1", m_data.fillStartColor.first );
        // ColorHelper::drawImGuiColorEdit4( "Particle Color B##2", m_data.fillEndColor.first );
        //
        // ImGui::Separator();
        //
        // ImGui::SliderFloat( "Thickness##2", &m_data.outlineThickness.first, 0.f, 25.f );
        //
        // ColorHelper::drawImGuiColorEdit4( "Particle Outline A##1", m_data.outlineStartColor.first );
        // ColorHelper::drawImGuiColorEdit4( "Particle Outline B##1", m_data.outlineEndColor.first );
        //
        // ImGui::SeparatorText( "Fade Easings" );



        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void drawAdjustmentMenu()
    {
      if ( ImGui::TreeNode( "Particle Adjust" ) )
      {
        // int32_t sides = m_data.pointCount.first;
        // if ( ImGui::SliderInt( "Sides##1", &sides, 3, 30 ) ) m_data.pointCount.first = sides;
        // ImGui::SliderFloat( "Radius##1", &m_data.radius.first, 0.0f, 150.0f );
        // ImGui::SliderFloat( "Timeout##1", &m_data.timeoutInSeconds.first, 0.015, 5.f );
        // ImGui::SliderFloat( "Boost##1", &m_data.boostVelocity.first, 0.f, 1.f );
        // ImGui::SliderFloat( "Velocity Size Mult##1", &m_data.velocitySizeMultiplier.first, 0.f, 50.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  protected:

    PipelineContext& m_ctx;

    TData m_data;

  };
}