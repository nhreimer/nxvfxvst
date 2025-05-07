#include "models/shader/ShockBloomShader.hpp"

namespace nx
{

    ShockBloomShader::ShockBloomShader(PipelineContext& context)
      : m_ctx( context )
    {
      if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
      {
        LOG_ERROR( "Failed to load shock bloom fragment shader" );
      }
      else
      {
        LOG_INFO( "Shock bloom shader loaded successfully" );
      }

      EXPAND_SHADER_VST_BINDINGS(SHOCK_BLOOM_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    ShockBloomShader::~ShockBloomShader()
    {
      m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
    }

    [[nodiscard]]
    nlohmann::json ShockBloomShader::serialize() const
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(SHOCK_BLOOM_SHADER_PARAMS)

      j[ "midiTriggers" ] = m_midiNoteControl.serialize();
      j[ "easing" ] = m_easing.serialize();
      return j;
    }

    void ShockBloomShader::deserialize(const nlohmann::json& j)
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(SHOCK_BLOOM_SHADER_PARAMS)

        m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
        m_easing.deserialize( j[ "easing" ] );
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    void ShockBloomShader::trigger( const Midi_t& midi )
    {
      if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
        m_easing.trigger();
    }

    void ShockBloomShader::drawMenu()
    {
      if ( ImGui::TreeNode( "Shock Bloom Options" ) )
      {
        ImGui::Checkbox( "Strobe Active##1", &m_data.isActive );

        const float oldCenterX = m_data.center.first.x;
        const float oldCenterY = m_data.center.first.y;

        EXPAND_SHADER_IMGUI(SHOCK_BLOOM_SHADER_PARAMS, m_data)

        if ( oldCenterX != m_data.center.first.x || oldCenterY != m_data.center.first.y )
        {
          const sf::Vector2f calibrated { m_data.center.first.x * static_cast< float >(m_ctx.globalInfo.windowSize.x),
                                          m_data.center.first.y * static_cast< float >(m_ctx.globalInfo.windowSize.y) };
          m_timedCursor.setPosition( calibrated );
        }

        ImGui::Separator();
        m_easing.drawMenu();

        ImGui::Separator();
        m_midiNoteControl.drawMenu();

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    [[nodiscard]]
    bool ShockBloomShader::isShaderActive() const { return m_data.isActive; }

    [[nodiscard]]
    sf::RenderTexture * ShockBloomShader::applyShader( const sf::RenderTexture * inputTexture )
    {
      m_outputTexture.ensureSize( inputTexture->getSize() );

      const float easing = m_easing.getEasing();
      const float radius = m_data.maxRadius.first * easing;
      const float alpha = easing * m_data.easingMultiplier.first;

      // Update uniforms
      m_shader.setUniform("resolution", sf::Vector2f(inputTexture->getSize()));
      m_shader.setUniform("center", m_data.center.first);
      m_shader.setUniform("radius", radius);
      m_shader.setUniform("thickness", m_data.thickness.first);
      m_shader.setUniform("color", m_data.color.first);
      m_shader.setUniform("intensity", m_data.intensity.first * alpha);
      m_shader.setUniform("innerTransparency", m_data.innerTransparency.first);

      // Fullscreen quad
      sf::RectangleShape fullscreen(sf::Vector2f(inputTexture->getSize()));
      fullscreen.setFillColor(sf::Color::White);

      m_outputTexture.clear(sf::Color::Transparent);
      m_outputTexture.draw(fullscreen, &m_shader);
      m_outputTexture.display();

      return m_blender.applyShader( inputTexture,
                                    m_outputTexture.get(),
                                    m_data.mixFactor.first );
    }

}