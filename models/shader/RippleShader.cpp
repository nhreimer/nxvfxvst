#include "models/shader/RippleShader.hpp"

namespace nx
{
    RippleShader::RippleShader( PipelineContext& context )
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load ripple fragment shader" );
      }
      else
      {
        LOG_INFO( "Ripple fragment shader loaded" );
        m_easing.setEasingType( E_TimeEasingType::E_Linear );
      }

      EXPAND_SHADER_VST_BINDINGS(RIPPLE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    RippleShader::~RippleShader()
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    ///////////////////////////////////////////////////////
    /// ISERIALIZABLE
    ///////////////////////////////////////////////////////

    nlohmann::json RippleShader::serialize() const
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(RIPPLE_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void RippleShader::deserialize(const nlohmann::json& j)
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(RIPPLE_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    void RippleShader::drawMenu()
    {
      if ( ImGui::TreeNode( "Ripple Options" ) )
      {
        ImGui::Checkbox( "Ripple Active##1", &m_data.isActive );

        const float oldCenterX = m_data.rippleCenterX.first;
        const float oldCenterY = m_data.rippleCenterY.first;

        ImGui::Checkbox( "Is Active##1", &m_data.isActive );
        EXPAND_SHADER_IMGUI(RIPPLE_SHADER_PARAMS, m_data)

        if ( oldCenterX != m_data.rippleCenterX.first || oldCenterY != m_data.rippleCenterY.first )
        {
          const sf::Vector2f calibrated { m_data.rippleCenterX.first * static_cast< float >(m_ctx.globalInfo.windowSize.x),
                                          m_data.rippleCenterY.first * static_cast< float >(m_ctx.globalInfo.windowSize.y) };
          m_timedCursor.setPosition( calibrated );
        }

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if ( !m_timedCursor.hasExpired() )
        m_timedCursor.drawPosition();
    }

    void RippleShader::trigger( const Midi_t& midi )
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    [[nodiscard]]
    bool RippleShader::isShaderActive() const { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture & RippleShader::applyShader( const sf::RenderTexture &inputTexture )
    {
      if ( m_outputTexture.getSize() != inputTexture.getSize() )
      {
        if ( !m_outputTexture.resize( inputTexture.getSize() ) )
        {
          LOG_ERROR( "failed to resize ripple texture" );
        }
      }

      constexpr float baseAmplitude = 0.005f;
      constexpr float maxPulseAmplitude = 0.03f;

      const float eased = m_easing.getEasing();
      m_data.amplitude.first = baseAmplitude + eased * maxPulseAmplitude;

      m_shader.setUniform( "texture", inputTexture.getTexture() );
      m_shader.setUniform( "resolution", sf::Vector2f( inputTexture.getSize() ) );
      m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );

      m_shader.setUniform( "rippleCenter", sf::Vector2f( m_data.rippleCenterX.first, m_data.rippleCenterY.first) );
      m_shader.setUniform( "amplitude", m_data.amplitude.first );     // 0.0f – 0.05f
      m_shader.setUniform( "frequency", m_data.frequency.first );     // 10.0f – 50.0f
      m_shader.setUniform( "speed", m_data.speed.first );             // 0.0f – 10.0f

      m_outputTexture.clear( sf::Color::Transparent );
      m_outputTexture.draw( sf::Sprite( inputTexture.getTexture() ), &m_shader );
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                              m_outputTexture,
                              m_data.mixFactor.first );
    }


}