#include "models/shader/ColorShader.hpp"

#include "helpers/SerialHelper.hpp"

#include "vst/params/VSTParamBindingManager.hpp"

namespace nx
{
  ColorShader::ColorShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load color fragment shader" );
    }
    else
    {
      LOG_DEBUG( "Color fragment shader loaded successfully" );
    }

    m_easing.setEasingType( E_TimeEasingType::E_Disabled );
    EXPAND_SHADER_VST_BINDINGS(COLOR_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  ColorShader::~ColorShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  [[nodiscard]]
  nlohmann::json ColorShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum(getType());
    EXPAND_SHADER_PARAMS_TO_JSON(COLOR_SHADER_PARAMS)
    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void ColorShader::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(COLOR_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void ColorShader::trigger(const Midi_t &midi)
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  void ColorShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Color Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(COLOR_SHADER_PARAMS, m_data)

      ImGui::SeparatorText( "Easings" );
      m_easing.drawMenu();

      ImGui::SeparatorText( "Midi Trigger" );
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  [[nodiscard]]
  sf::RenderTexture * ColorShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    const auto easing = m_easing.getEasing();

    m_shader.setUniform("u_texture", inputTexture->getTexture());
    m_shader.setUniform( "u_brightness", m_data.brightness.first * easing );
    m_shader.setUniform( "u_saturation", m_data.saturation.first * easing );
    m_shader.setUniform( "u_contrast", m_data.contrast.first );
    m_shader.setUniform( "u_hueShift", m_data.hueShift.first );
    m_shader.setUniform( "u_gain", m_data.colorGain.first );

    m_outputTexture.clear(sf::Color::Transparent);
    m_outputTexture.draw(sf::Sprite( inputTexture->getTexture() ), &m_shader);
    m_outputTexture.display();

    //return m_outputTexture;
    return m_blender.applyShader( inputTexture,
                                  m_outputTexture.get(),
                                  m_data.mixFactor.first );
  }

}