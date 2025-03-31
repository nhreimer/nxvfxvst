#pragma once

#include "shapes/GradientLine.hpp"
#include "models/data/ParticleLineData_t.hpp"

namespace nx
{

  class ParticleSequentialLineModifier final : public IParticleModifier
  {
  public:

    explicit ParticleSequentialLineModifier( const GlobalInfo_t& winfo )
      : m_winfo( winfo )
    {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Sequential Lines" ) )
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
          LOG_ERROR( "failed to resize sequential line texture" );
        else
          LOG_INFO( "successfully resized sequential line texture" );
      }

      m_outputTexture.clear();

      for ( int i = 0; i < particles.size(); ++i )
      {
        if ( m_data.useConnectors && i > 0 )
        {
          GradientLine line;
          line.setStart( particles[ i - 1 ].shape.getPosition() );
          line.setEnd( particles[ i ].shape.getPosition() );
          line.setWidth( m_data.lineThickness );

          if ( particles[ i ].timeLeft > particles[ i - 1 ].timeLeft )
          {
            line.setGradient( particles[ i ].shape.getFillColor(),
                             particles[ i - 1 ].shape.getFillColor() );
          }
          else
          {
            line.setGradient( particles[ i - 1 ].shape.getFillColor(),
                              particles[ i ].shape.getFillColor() );
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