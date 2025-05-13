#include "models/ParticleBehaviorPipeline.hpp"

#include "models/particle/behavior/RadialSpreaderBehavior.hpp"
#include "models/particle/behavior/FreeFallBehavior.hpp"
#include "models/particle/behavior/JitterBehavior.hpp"
#include "models/particle/behavior/ColorMorphBehavior.hpp"
#include "models/particle/behavior/MagneticBehavior.hpp"

namespace nx
{

  nlohmann::json ParticleBehaviorPipeline::savePipeline() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( const auto& behavior : m_particleBehaviors )
      j.push_back( behavior->serialize() );

    return j;
  }

  void ParticleBehaviorPipeline::loadPipeline( const nlohmann::json& j )
  {
    m_particleBehaviors.clear();
    for ( const auto& data : j )
    {
      if ( !j.contains( "type" ) )
        continue;

      const auto type =
        SerialHelper::deserializeEnum< E_BehaviorType >( data.value("type", "" ) );
      switch ( type )
      {
        case E_BehaviorType::E_JitterBehavior:
          deserializeBehavior< JitterBehavior >( data );
          break;

        case E_BehaviorType::E_FreeFallBehavior:
          deserializeBehavior< FreeFallBehavior >( data );
          break;

        case E_BehaviorType::E_RadialSpreaderBehavior:
          deserializeBehavior< RadialSpreaderBehavior >( data );
          break;

        case E_BehaviorType::E_ColorMorphBehavior:
          deserializeBehavior< ColorMorphBehavior >( data );
          break;

        case E_BehaviorType::E_MagneticBehavior:
          deserializeBehavior< MagneticAttractorBehavior >( data );
          break;

        default:
          LOG_ERROR( "unable to deserialize modifier type" );
          break;
      }
    }
  }

  void ParticleBehaviorPipeline::applyOnSpawn( IParticle * p,
                                               const Midi_t& midi,
                                               const ParticleData_t& particleData,
                                               const sf::Vector2f& position ) const
  {
    for ( const auto& behavior : m_particleBehaviors )
      behavior->applyOnSpawn( p, midi, particleData, position );
  }

  void ParticleBehaviorPipeline::applyOnUpdate( IParticle * p,
                                                const sf::Time& deltaTime,
                                                const ParticleData_t& particleData,
                                                const sf::Vector2f& position ) const
  {
    for ( const auto& behavior : m_particleBehaviors )
      behavior->applyOnUpdate( p, deltaTime, particleData, position );
  }

  void ParticleBehaviorPipeline::drawMenu()
  {
    drawBehaviorsAvailable();
    drawBehaviorPipelineMenu();
  }

  void ParticleBehaviorPipeline::drawBehaviorPipelineMenu()
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

  void ParticleBehaviorPipeline::drawBehaviorsAvailable()
  {
    if ( ImGui::TreeNode( "Behaviors Available" ) )
    {
      if ( ImGui::Button( "Radial Spread##1" ) )
        createBehavior< RadialSpreaderBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Free Fall##1" ) )
        createBehavior< FreeFallBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Jitter##1" ) )
        createBehavior< JitterBehavior >();

      ImGui::SameLine();
      if ( ImGui::Button( "Color Morph##1" ) )
        createBehavior< ColorMorphBehavior >();

      if ( ImGui::Button( "Magnetic##1" ) )
        createBehavior< MagneticAttractorBehavior >();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }
}