#include "models/particle/layout/LSystemCurveLayout.hpp"

namespace nx
{

  [[nodiscard]]
  nlohmann::json LSystemCurveLayout::serialize() const
  {
    nlohmann::json j =
    {
      { "type", SerialHelper::serializeEnum( getType() ) }
    };

    j[ "depth" ] = m_data.depth;
    j[ "turnAngle" ] = m_data.turnAngle;
    j[ "segmentLength" ] = m_data.segmentLength;
    j[ "initialAngleDeg" ] = m_data.initialAngleDeg;
    j[ "spreadFactor" ] = m_data.spreadFactor;
    j[ "depthFactor" ] = m_data.depthFactor;
    j[ "stepsPerNote" ] = m_data.stepsPerNote;
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    j[ "particleGenerator" ] = m_particleGenerator->serialize();
    return j;
  }

  void LSystemCurveLayout::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.depth = j["depth"];
      m_data.turnAngle = j["turnAngle"];
      m_data.segmentLength = j["segmentLength"];
      m_data.initialAngleDeg = j["initialAngleDeg"];
      m_data.spreadFactor = j["spreadFactor"];
      m_data.depthFactor = j["depthFactor"];
      m_data.stepsPerNote = j["stepsPerNote"];
    }
    else
    {
      LOG_DEBUG( "failed to find type for layout {}", SerialHelper::serializeEnum( getType() ) );
    }

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j["behaviors"] );
    if ( j.contains( "particleGenerator" ) )
      m_particleGenerator->deserialize( j.at( "particleGenerator" ) );
  }

  void LSystemCurveLayout::drawMenu()
  {
    ImGui::Text("Particles: %d", m_particles.size());
    ImGui::Separator();
    if (ImGui::TreeNode("L-System Particle Layout"))
    {
      m_particleGenerator->drawMenu();

      ImGui::Separator();
      ImGui::SliderInt("Depth", &m_data.depth, 1, 12);
      ImGui::SliderFloat("Segment Length", &m_data.segmentLength, 5.f, 100.f);
      ImGui::SliderFloat("Turn Angle (deg)", &m_data.turnAngle, 1.f, 90.f);
      ImGui::SliderFloat("Initial Angle", &m_data.initialAngleDeg, -180.f, 180.f);
      ImGui::SliderFloat( "Spread out", &m_data.spreadFactor, 1.f, 5.f );
      ImGui::SliderFloat( "Tightness", &m_data.depthFactor, 0.f, 5.f );
      ImGui::SliderInt( "Steps per Note", &m_data.stepsPerNote, 1, 5 );

      ImGui::SeparatorText( "Branch Mode" );
      if ( ImGui::RadioButton( "Both", m_data.m_branchMode == E_LSystemBranchMode::E_Both ) )
        m_data.m_branchMode = E_LSystemBranchMode::E_Both;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Left", m_data.m_branchMode == E_LSystemBranchMode::E_LeftOnly ) )
        m_data.m_branchMode = E_LSystemBranchMode::E_LeftOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Right", m_data.m_branchMode == E_LSystemBranchMode::E_RightOnly ) )
        m_data.m_branchMode = E_LSystemBranchMode::E_RightOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Midi Pitch", m_data.m_branchMode == E_LSystemBranchMode::E_MidiPitch ) )
        m_data.m_branchMode = E_LSystemBranchMode::E_MidiPitch;

      ImGui::Separator();
      m_behaviorPipeline.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void LSystemCurveLayout::addMidiEvent( const Midi_t &midiEvent )
  {
    // 127 - 21 = full pitch range
    if (m_lsystemStack.empty())
    {
      // Start a new tree!
      const auto offsetX = midiEvent.pitch / 106.f;
      const sf::Vector2f position = {
        m_ctx.globalInfo.windowSize.x * offsetX,
        m_ctx.globalInfo.windowSize.y * offsetX
      };

      m_currentDepth = 1;
      m_lsystemStack.clear();

      m_lsystemStack.push_back({
        position,
        m_data.initialAngleDeg,
        m_data.depth,
        midiEvent
      });
    }

    for (int i = 0; i < m_data.stepsPerNote && !m_lsystemStack.empty(); ++i)
    {
      auto state = m_lsystemStack.back();
      m_lsystemStack.pop_back();

      expandLSystemStep( midiEvent, state);
    }
  }

  void LSystemCurveLayout::expandLSystemStep(const Midi_t &midiEvent,
                                             const LSystemState_t& state )
  {
    if ( state.depth <= 0 )
    {
      // Final particle placement
      auto * p = m_particles.emplace_back(
      m_particleGenerator->createParticle(
        midiEvent,
        m_ctx.globalInfo.elapsedTimeSeconds ) );

      notifyBehaviorOnSpawn( p, midiEvent );
      return;
    }

    // Extend to the next segment
    const float tightness = 1.f + ( m_data.depth - m_currentDepth ) * m_data.depthFactor;
    const sf::Vector2f dir = MathHelper::polarToCartesian(state.angleDeg,
                              m_data.segmentLength * m_data.spreadFactor * tightness);
    const sf::Vector2f newPos = ( state.position + dir );

    switch (m_data.m_branchMode)
    {
      case E_LSystemBranchMode::E_Both:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_LSystemBranchMode::E_LeftOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_LSystemBranchMode::E_RightOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_LSystemBranchMode::E_MidiPitch:
        switch (state.midiNote.pitch % 3)
        {
          case 0:
            m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
            m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
            break;
          case 1:
            m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
            break;
          case 2:
            m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
            break;

          default: break;
        }
        break;

      default: break;
    }
  }

} // namespace nx
