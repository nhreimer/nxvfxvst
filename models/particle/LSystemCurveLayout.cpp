#include "models/particle/LSystemCurveLayout.hpp"

namespace nx
{

  LSystemCurveLayout::~LSystemCurveLayout()
  {
    for ( const auto * particle : m_particles )
      delete particle;
  }

  [[nodiscard]]
  nlohmann::json LSystemCurveLayout::serialize() const
  {
    auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
    j[ "depth" ] = m_data.depth;
    j[ "turnAngle" ] = m_data.turnAngle;
    j[ "segmentLength" ] = m_data.segmentLength;
    j[ "initialAngleDeg" ] = m_data.initialAngleDeg;
    j[ "behaviors" ] = m_behaviorPipeline.savePipeline();
    return j;
  }

  void LSystemCurveLayout::deserialize(const nlohmann::json &j)
  {
    ParticleHelper::deserialize( m_data, SerialHelper::serializeEnum( getType() ) );
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.depth = j["depth"];
      m_data.turnAngle = j["turnAngle"];
      m_data.segmentLength = j["segmentLength"];
      m_data.initialAngleDeg = j["initialAngleDeg"];
    }
    else
    {
      LOG_DEBUG( "failed to find type for layout {}", SerialHelper::serializeEnum( getType() ) );
    }

    if ( j.contains( "behaviors" ) )
      m_behaviorPipeline.loadPipeline( j["behaviors"] );
  }

  void LSystemCurveLayout::drawMenu()
  {
    ImGui::Text("Particles: %d", m_particles.size());
    ImGui::Separator();
    if (ImGui::TreeNode("L-System Particle Layout"))
    {
      ParticleHelper::drawMenu(m_data );

      ImGui::Separator();
      ImGui::SliderInt("Depth", &m_data.depth, 1, 12);
      ImGui::SliderFloat("Segment Length", &m_data.segmentLength, 5.f, 100.f);
      ImGui::SliderFloat("Turn Angle (deg)", &m_data.turnAngle, 1.f, 90.f);
      ImGui::SliderFloat("Initial Angle", &m_data.initialAngleDeg, -180.f, 180.f);
      ImGui::SliderInt( "Steps per Note", &m_data.stepsPerNote, 1, 5 );

      ImGui::SeparatorText( "Branch Mode" );
      if ( ImGui::RadioButton( "Both", m_data.m_branchMode == E_BranchMode::E_Both ) )
        m_data.m_branchMode = E_BranchMode::E_Both;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Left", m_data.m_branchMode == E_BranchMode::E_LeftOnly ) )
        m_data.m_branchMode = E_BranchMode::E_LeftOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Right", m_data.m_branchMode == E_BranchMode::E_RightOnly ) )
        m_data.m_branchMode = E_BranchMode::E_RightOnly;

      ImGui::SameLine();
      if ( ImGui::RadioButton( "Midi Pitch", m_data.m_branchMode == E_BranchMode::E_MidiPitch ) )
        m_data.m_branchMode = E_BranchMode::E_MidiPitch;

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

    // How many elements to expand per MIDI note? (adjustable)
    //constexpr int stepsPerNote = 1;

    for (int i = 0; i < m_data.stepsPerNote && !m_lsystemStack.empty(); ++i)
    {
      auto state = m_lsystemStack.back();
      m_lsystemStack.pop_back();

      expandLSystemStep(state);
    }
  }

  void LSystemCurveLayout::update( const sf::Time &deltaTime )
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

  void LSystemCurveLayout::expandLSystemStep(const LSystemState_t& state)
  {
    if (state.depth <= 0)
    {
      // Final particle placement
      auto* p = m_particles.emplace_back(new TimedParticle_t());
      p->shape.setRadius(m_data.radius);

      if (p->shape.getPointCount() != m_data.shapeSides)
        p->shape.setPointCount(m_data.shapeSides);

      p->shape.setOutlineThickness(m_data.outlineThickness);
      p->shape.setOutlineColor(m_data.outlineColor);
      p->shape.setOrigin(p->shape.getGlobalBounds().size / 2.f);
      p->shape.setPosition(state.position);
      p->spawnTime = m_ctx.globalInfo.elapsedTimeSeconds;
      p->initialColor = m_data.startColor;
      m_behaviorPipeline.applyOnSpawn(p, state.midiNote);
      return;
    }

    // Extend to next segment
    const sf::Vector2f dir = MathHelper::polarToCartesian(state.angleDeg, m_data.segmentLength);
    const sf::Vector2f newPos = state.position + dir;

    switch (m_data.m_branchMode)
    {
      case E_BranchMode::E_Both:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_BranchMode::E_LeftOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg + m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_BranchMode::E_RightOnly:
        m_lsystemStack.push_back({ newPos, state.angleDeg - m_data.turnAngle, state.depth - 1, state.midiNote });
        break;

      case E_BranchMode::E_MidiPitch:
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
