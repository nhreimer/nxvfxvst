#pragma once

#include "InterfaceTypes.hpp"

namespace nx
{

  struct IParticleBehavior
  {
    virtual ~IParticleBehavior() = default;
    virtual void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) = 0;
    virtual void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) = 0;
    virtual void drawMenu() = 0;
    virtual E_BehaviorType getType() const = 0;
  };

  class RadialSpreaderBehavior final : public IParticleBehavior
  {
  public:
    explicit RadialSpreaderBehavior(const GlobalInfo_t& info)
      : m_globalInfo(info)
    {}

    E_BehaviorType getType() const override { return E_RadialSpreaderBehavior; }

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) override
    {
      sf::Vector2f pos = p->shape.getPosition();
      const sf::Vector2f dir =
        ( pos - m_globalInfo.windowHalfSize ) * ( m_globalInfo.elapsedTimeSeconds - p->spawnTime );

      // Avoid NaNs if particle spawns directly at center
      if (dir.x == 0.f && dir.y == 0.f) return;

      pos = m_globalInfo.windowHalfSize + dir * m_spreadMultiplier;
      p->shape.setPosition(pos);
    }

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) override
    {
      sf::Vector2f baseDir = p->originalPosition - m_globalInfo.windowHalfSize;

      float elapsed = m_globalInfo.elapsedTimeSeconds - p->spawnTime;
      float pulse = std::sin(elapsed * m_speed) * 0.5f + 0.5f; // oscillates between [0, 1]

      sf::Vector2f animatedPos = m_globalInfo.windowHalfSize + baseDir * (1.f + m_spreadMultiplier * pulse);
      p->shape.setPosition(animatedPos);
    }

    void drawMenu() override
    {
      ImGui::SliderFloat( "Spread Multiplier", &m_spreadMultiplier, 0.0f, 5.0f );
      ImGui::SliderFloat( "Spread Speed", &m_speed, 0.0f, 5.0f );
    }

  private:
    const GlobalInfo_t& m_globalInfo;
    float m_spreadMultiplier = 1.5f;
    float m_speed = 0.5f;
  };

  class ParticleBehaviorPipeline final
  {
  public:

    explicit ParticleBehaviorPipeline( const GlobalInfo_t& info )
      : m_globalInfo( info )
    {}

    void applyOnSpawn( TimedParticle_t * p, const Midi_t& midi ) const
    {
      for ( const auto& behavior : m_particleBehaviors )
        behavior->applyOnSpawn( p, midi );
    }

    void applyOnUpdate( TimedParticle_t * p, const sf::Time& deltaTime ) const
    {
      for ( const auto& behavior : m_particleBehaviors )
        behavior->applyOnUpdate( p, deltaTime );
    }

    void drawMenu()
    {
      drawBehaviorsAvailable();
      drawBehaviorPipelineMenu();
    }

  private:

    void drawBehaviorPipelineMenu()
    {
      ImGui::Separator();
      ImGui::Text( "Behaviors: %d", m_particleBehaviors.size() );

      int deletePos = -1;
      int swapA = -1;
      int swapB = -1;

      if ( ImGui::TreeNode( "Active Behaviors" ) )
      {
        for ( int i = 0; i < m_particleBehaviors.size(); ++i )
        {
          ImGui::PushID( i );

          if ( i > 0 )
            ImGui::Separator();

          if ( ImGui::Button( "x" ) )
            deletePos = i;
          else
          {
            ImGui::SameLine();
            m_particleBehaviors[ i ]->drawMenu();

            if ( ImGui::Button( "u" ) )
            {
              swapA = i;
              swapB = i - 1;
            }

            ImGui::SameLine();

            if ( ImGui::Button( "d" ) )
            {
              swapA = i;
              swapB = i + 1;
            }
          }
          ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( deletePos > -1 )
        m_particleBehaviors.erase( m_particleBehaviors.begin() + deletePos );
      else if ( swapA > -1 && swapB > -1 && swapA < m_particleBehaviors.size() && swapB < m_particleBehaviors.size() )
        std::swap( m_particleBehaviors[ swapA ], m_particleBehaviors[ swapB ] );
    }

    void drawBehaviorsAvailable()
    {
      if ( ImGui::TreeNode( "Behaviors Available" ) )
      {
        if ( ImGui::Button( "Radial Spread##1" ) )
          createBehavior< RadialSpreaderBehavior >();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    template < typename T >
    IParticleBehavior * createBehavior()
    {
      auto& behavior = m_particleBehaviors.emplace_back< std::unique_ptr< T > >(
        std::make_unique< T >( m_globalInfo ) );
      return behavior.get();
    }

  private:

    const GlobalInfo_t& m_globalInfo;
    std::vector< std::unique_ptr< IParticleBehavior > > m_particleBehaviors;
  };

}