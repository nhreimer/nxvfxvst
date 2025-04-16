#pragma once

#include "models/particle/ParticleConsumer.hpp"
#include "models/particle/ParticleLayoutBase.hpp"

namespace nx
{

  struct TestParticleLayoutData_t : public ParticleLayoutData_t
  {};

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class TestParticleLayout final : public ParticleLayoutBase< TestParticleLayoutData_t >
  {
  public:
    explicit TestParticleLayout(const GlobalInfo_t &globalInfo) : ParticleLayoutBase(globalInfo) {}

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      auto j = ParticleHelper::serialize(m_data, SerialHelper::serializeEnum(getType()));
      // TODO: insert components of this layout here
      j[ "behaviors" ] = m_behaviorPipeline.saveModifierPipeline();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      ParticleHelper::deserialize(m_data, j);

      // TODO: insert components of this layout here

      if (j.contains("behaviors"))
        m_behaviorPipeline.loadModifierPipeline(j.at("behaviors"));
    }

    E_LayoutType getType() const override { return E_LayoutType::E_TestLayout; }

    void drawMenu() override
    {
      ImGui::Text("Particles: %d", m_particles.size());
      ImGui::Separator();
      if (ImGui::TreeNode("Test Particle Layout"))
      {
        ParticleHelper::drawMenu(*reinterpret_cast< ParticleLayoutData_t * >(&m_data));

        ImGui::Separator();
        m_behaviorPipeline.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void addMidiEvent(const Midi_t &midiEvent) override
    {

    }

  private:

  private:

  };

} // namespace nx
