#pragma once

#include "models/particle/behavior/RadialSpreaderBehavior.hpp"
#include "models/particle/behavior/FreeFallBehavior.hpp"

namespace nx
{
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

        ImGui::SameLine();
        if ( ImGui::Button( "Free Fall##1" ) )
          createBehavior< FreeFallBehavior >();

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