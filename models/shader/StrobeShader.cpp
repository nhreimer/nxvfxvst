#include "models/shader/StrobeShader.hpp"

namespace nx
{

  StrobeShader::StrobeShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load strobe fragment shader" );
    }
    else
    {
      LOG_INFO( "Strobe shader loaded successfully" );
    }

    EXPAND_SHADER_VST_BINDINGS(STROBE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  StrobeShader::~StrobeShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json StrobeShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(STROBE_SHADER_PARAMS)

    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void StrobeShader::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(STROBE_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void StrobeShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Strobe Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(STROBE_SHADER_PARAMS, m_data)

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void StrobeShader::trigger( const Midi_t &midi )
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  [[nodiscard]]
  bool StrobeShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * StrobeShader::applyShader( const sf::RenderTexture * inputTexture )
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    m_shader.setUniform( "texture", inputTexture->getTexture() );
    m_shader.setUniform("flashAmount", m_data.flashAmount.first * m_easing.getEasing()); // or assigned easing
    m_shader.setUniform("flashColor", sf::Glsl::Vec4(m_data.flashColor.first));

    m_outputTexture.clear( sf::Color::Transparent );
    m_outputTexture.draw( sf::Sprite( inputTexture->getTexture() ), &m_shader );
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                            m_outputTexture.get(),
                            m_data.mixFactor.first );
  }
}