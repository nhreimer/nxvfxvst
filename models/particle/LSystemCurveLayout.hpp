#pragma once

#include "models/particle/ParticleConsumer.hpp"

namespace nx
{

  enum class E_BranchMode : int8_t
  {
    E_Both,
    E_LeftOnly,
    E_RightOnly,
    E_MidiPitch
  };

  struct LSystemCurveLayoutData_t : public ParticleLayoutData_t
  {
    int depth = 4; // number of recursive steps
    float turnAngle = 25.f; // degrees turned on L/R
    float segmentLength = 20.f; // length of each forward step
    float initialAngleDeg = 0.f;
    E_BranchMode m_branchMode = E_BranchMode::E_Both;
  };

  /// this is useful for adding time-based effects to a screen without
  /// having particles on the screen.
  class LSystemCurveLayout final : public IParticleLayout
  {
  public:
    explicit LSystemCurveLayout(const GlobalInfo_t &globalInfo)
      : m_globalInfo( globalInfo ),
        m_behaviorPipeline( globalInfo )
    {}

    ~LSystemCurveLayout() override
    {
      for ( const auto * particle : m_particles )
        delete particle;
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      auto j = ParticleHelper::serialize( m_data, SerialHelper::serializeEnum( getType() ) );
      j[ "depth" ] = m_data.depth;
      j[ "turnAngle" ] = m_data.turnAngle;
      j[ "segmentLength" ] = m_data.segmentLength;
      j[ "initialAngleDeg" ] = m_data.initialAngleDeg;
      j[ "behaviors" ] = m_behaviorPipeline.saveModifierPipeline();
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      ParticleHelper::deserialize( m_data, SerialHelper::serializeEnum( getType() ) );
      if ( j.contains( SerialHelper::serializeEnum( getType() ) ) )
      {
        m_data.depth = j["depth"];
        m_data.turnAngle = j["turnAngle"];
        m_data.segmentLength = j["segmentLength"];
        m_data.initialAngleDeg = j["initialAngleDeg"];
      }

      if ( j.contains( "behaviors" ) )
        m_behaviorPipeline.loadModifierPipeline( j["behaviors"] );
    }

    E_LayoutType getType() const override { return E_LayoutType::E_LSystemCurveLayout; }

    void drawMenu() override
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

        ImGui::Text( "Branch Mode" );
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

    void addMidiEvent( const Midi_t &midiEvent ) override
    {
      drawLSystem( m_globalInfo.windowHalfSize,
                   m_data.initialAngleDeg,
                   m_data.depth,
                   midiEvent );
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
    void drawLSystem( const sf::Vector2f position,
                      const float angleDeg,
                      const int depth,
                      const Midi_t & midiNote )
    {
      if ( depth <= 0 )
      {
        // Place particle at final location
        auto * p = m_particles.emplace_back( new TimedParticle_t() );
        p->shape.setRadius(m_data.radius);

        if ( p->shape.getPointCount() != m_data.shapeSides )
          p->shape.setPointCount( m_data.shapeSides );

        p->shape.setOutlineThickness( m_data.outlineThickness );
        p->shape.setOutlineColor( m_data.outlineColor );

        p->shape.setOrigin( p->shape.getGlobalBounds().size / 2.f );
        p->shape.setPosition(position);
        p->spawnTime = m_globalInfo.elapsedTimeSeconds;
        p->initialColor = m_data.startColor;
        m_behaviorPipeline.applyOnSpawn( p, midiNote );
        return;
      }

      // Forward segment
      const sf::Vector2f dir = MathHelper::polarToCartesian(angleDeg, m_data.segmentLength);
      const sf::Vector2f newPos = position + dir;

      switch ( m_data.m_branchMode )
      {
        case E_BranchMode::E_Both:
          drawLSystem( newPos, angleDeg + m_data.turnAngle, depth - 1, midiNote );
          drawLSystem( newPos, angleDeg - m_data.turnAngle, depth - 1, midiNote );
          break;

        case E_BranchMode::E_LeftOnly:
          drawLSystem( newPos, angleDeg + m_data.turnAngle, depth - 1, midiNote );
          break;

        case E_BranchMode::E_RightOnly:
          drawLSystem( newPos, angleDeg - m_data.turnAngle, depth - 1, midiNote );
          break;

        case E_BranchMode::E_MidiPitch:
        {
          switch ( midiNote.pitch % 3 )
          {
            case 0:
              drawLSystem( newPos, angleDeg + m_data.turnAngle, depth - 1, midiNote );
              drawLSystem( newPos, angleDeg - m_data.turnAngle, depth - 1, midiNote );
              break;
            case 1:
              drawLSystem( newPos, angleDeg + m_data.turnAngle, depth - 1, midiNote );
              break;
            case 2:
              drawLSystem( newPos, angleDeg - m_data.turnAngle, depth - 1, midiNote );
              break;
            default:
              break;
          }
        }
      }
    }

  private:
    const GlobalInfo_t & m_globalInfo;
    LSystemCurveLayoutData_t m_data;
    ParticleBehaviorPipeline m_behaviorPipeline;

    std::deque< TimedParticle_t* > m_particles;
  };

} // namespace nx
