#pragma once

#include "helpers/MenuHelper.hpp"

#include "models/data/ParticleLineData_t.hpp"

#include "shapes/GradientLine.hpp"

namespace nx
{

  class ParticleFullMeshLineModifier final : public IParticleModifier
  {
  public:

    explicit ParticleFullMeshLineModifier( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Full Mesh Lines" ) )
      {
        ImGui::Checkbox( "Connect##1", &m_data.useConnectors );
        ImGui::SliderFloat( "Thickness##1", &m_data.lineThickness, 1.f, 100.f, "Thickness %0.2f" );

        MenuHelper::drawBlendOptions( m_data.blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void update( const sf::Time &deltaTime ) override
    {}

    [[nodiscard]]
    sf::RenderTexture& modifyParticles( const ParticleLayoutData_t& particleLayoutData,
                                        std::deque< TimedParticle_t >& particles ) override
    {
      if ( m_outputTexture.getSize() != m_winfo.windowSize )
      {
        if ( !m_outputTexture.resize( m_winfo.windowSize ) )
          LOG_ERROR( "failed to resize full-mesh particle texture" );
        else
          LOG_INFO( "successfully resized full-mesh particle texture" );
      }

      m_outputTexture.clear();

      for ( int i = 0; i < particles.size(); ++i )
      {
        for ( int y = i + 1; y < particles.size(); ++y )
        {
          GradientLine line;
          line.setStart( particles[ i ].shape.getPosition() );
          line.setEnd( particles[ y ].shape.getPosition() );
          line.setWidth( m_data.lineThickness );

          if ( particles[ y ].timeLeft > particles[ i ].timeLeft )
          {
            line.setGradient( particles[ y ].shape.getFillColor(),
                             particles[ i ].shape.getFillColor() );
          }
          else
          {
            line.setGradient( particles[ i ].shape.getFillColor(),
                              particles[ y ].shape.getFillColor() );
          }
          m_outputTexture.draw( line, m_data.blendMode );

        }

        m_outputTexture.draw( particles[ i ].shape, particleLayoutData.blendMode );
      }

      m_outputTexture.display();
      return m_outputTexture;
    }

  private:

    const GlobalInfo_t& m_winfo;
    sf::RenderTexture m_outputTexture;

    ParticleLineData_t m_data;

  };

}