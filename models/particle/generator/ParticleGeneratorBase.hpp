#pragma once

#include "models/IParticle.hpp"
#include "models/IParticleGenerator.hpp"

#include "helpers/ColorHelper.hpp"
#include "helpers/SerialHelper.hpp"

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

    void drawMenu() override
    {
      drawAppearanceMenu();
      drawAdjustmentMenu();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return
      {
        { "type", getType() },
        { "fillStartColor", SerialHelper::convertColorToJson( m_data.fillStartColor ) },
        { "fillEndColor", SerialHelper::convertColorToJson( m_data.fillEndColor ) },
        { "outlineStartColor", SerialHelper::convertColorToJson( m_data.outlineStartColor ) },
        { "outlineEndColor", SerialHelper::convertColorToJson( m_data.outlineEndColor ) },
        { "outlineThickness", m_data.outlineThickness },
        { "radius", m_data.radius },
        { "pointCount", m_data.pointCount },
        { "timeoutInSeconds", m_data.timeoutInSeconds },
        { "boostVelocity", m_data.boostVelocity },
        { "velocitySizeMultiplier", m_data.velocitySizeMultiplier }
        //{ "blendMode", SerialHelper::convertBlendModeToString( m_data.blendMode ) }
      };
    }

    void deserialize(const nlohmann::json &j) override
    {
      m_data.fillStartColor = SerialHelper::convertColorFromJson(j.at("fillStartColor"), sf::Color::White);
      m_data.fillEndColor = SerialHelper::convertColorFromJson(j.at("fillEndColor"), sf::Color::Black);
      m_data.outlineStartColor = SerialHelper::convertColorFromJson(j.at("outlineStartColor"), sf::Color::White);
      m_data.outlineEndColor = SerialHelper::convertColorFromJson(j.at("outlineEndColor"), sf::Color::White);
      m_data.outlineThickness = j.value("outlineThickness", 0.f);
      m_data.radius = j.value("radius", 30.f);
      m_data.pointCount = j.value("pointCount", 30);
      m_data.timeoutInSeconds = j.value("timeoutInSeconds", 1.f);
      m_data.boostVelocity = j.value("boostVelocity", 0.f);
      m_data.velocitySizeMultiplier = j.value("velocitySizeMultiplier", 0.f);
      //m_data.blendMode = SerialHelper::convertBlendModeFromString(j.value("blendMode", "None"));
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
      // particle->setRadius( m_data.radius +
      //                      m_data.velocitySizeMultiplier * midiEvent.velocity );

      const auto initialStartColor = ColorHelper::getColorPercentage(
        m_data.fillStartColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      const auto initialEndColor = ColorHelper::getColorPercentage(
        m_data.fillEndColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      particle->setColorPattern( initialStartColor, initialEndColor );
      // particle->setOutlineThickness( m_data.outlineThickness );

      const auto initialOutlineStartColor = ColorHelper::getColorPercentage(
        m_data.outlineStartColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      const auto initialOutlineEndColor = ColorHelper::getColorPercentage(
        m_data.outlineEndColor,
        std::min( midiEvent.velocity + m_data.boostVelocity, 1.f ) );

      particle->setOutlineColorPattern( initialOutlineStartColor, initialOutlineEndColor );

      particle->setOrigin( particle->getGlobalBounds().size / 2.f );
      particle->setColorPattern( m_data.fillStartColor, m_data.fillEndColor );

      particle->setExpirationTimeInSeconds( m_data.timeoutInSeconds + particle->getSpawnTimeInSeconds() );
    }

  private:

    void drawAppearanceMenu()
    {
      if ( ImGui::TreeNode( "Particle Appearance" ) )
      {

        ColorHelper::drawImGuiColorEdit4( "Particle Color A##1", m_data.fillStartColor );
        ColorHelper::drawImGuiColorEdit4( "Particle Color B##2", m_data.fillEndColor );

        ImGui::SliderInt( "Color Start Offset", &m_data.colorVertexStartOffset, 0, m_data.pointCount );
        ImGui::SliderInt( "Color Interval", &m_data.colorVertexInterval, 1, m_data.pointCount );

        ImGui::Separator();

        ImGui::SliderFloat( "Thickness##2", &m_data.outlineThickness, 0.f, 25.f );

        ColorHelper::drawImGuiColorEdit4( "Particle Outline A##1", m_data.outlineStartColor );
        ColorHelper::drawImGuiColorEdit4( "Particle Outline B##1", m_data.outlineEndColor );

        ImGui::SeparatorText( "Fade Easings" );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void drawAdjustmentMenu()
    {
      if ( ImGui::TreeNode( "Particle Adjust" ) )
      {
        int32_t sides = m_data.pointCount;
        if ( ImGui::SliderInt( "Sides##1", &sides, 3, 30 ) ) m_data.pointCount = sides;
        ImGui::SliderFloat( "Radius##1", &m_data.radius, 0.0f, 500.0f );
        ImGui::SliderFloat( "Timeout##1", &m_data.timeoutInSeconds, 0.015, 5.f );
        ImGui::SliderFloat( "Boost##1", &m_data.boostVelocity, 0.f, 1.f );
        ImGui::SliderFloat( "Velocity Size Mult##1", &m_data.velocitySizeMultiplier, 0.f, 50.f );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    TData m_data;
  };
}