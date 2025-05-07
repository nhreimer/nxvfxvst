#include "models/shader/KaleidoscopeShader.hpp"

#include "helpers/CommonHeaders.hpp"

namespace nx
{
  KaleidoscopeShader::KaleidoscopeShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load kaleidoscope fragment shader" );
    }
    else
    {
      LOG_INFO( "Kaleidoscope fragment shader loaded" );
    }

    EXPAND_SHADER_VST_BINDINGS(KALEIDOSCOPE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  KaleidoscopeShader::~KaleidoscopeShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json KaleidoscopeShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum(getType());
    EXPAND_SHADER_PARAMS_TO_JSON(KALEIDOSCOPE_SHADER_PARAMS)
    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void KaleidoscopeShader::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(KALEIDOSCOPE_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void KaleidoscopeShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Cosmic-Kaleidoscope Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(KALEIDOSCOPE_SHADER_PARAMS, m_data)

      ImGui::SeparatorText( "Easings" );
      m_easing.drawMenu();
      ImGui::SeparatorText( "Midi Triggers" );
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursor.hasExpired() )
      m_timedCursor.drawPosition();
  }

  void KaleidoscopeShader::trigger( const Midi_t& midi )
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
    {
      m_easing.trigger();
    }
  }

  [[nodiscard]]
  bool KaleidoscopeShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * KaleidoscopeShader::applyShader(
    const sf::RenderTexture * inputTexture )
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    m_shader.setUniform( "u_time", m_easing.getEasing() );

    m_shader.setUniform("u_texture", inputTexture->getTexture());
    m_shader.setUniform("u_intensity", m_data.masterGain.first);
    m_shader.setUniform("u_resolution", sf::Vector2f(inputTexture->getSize()));

    // Custom control knobs
    m_shader.setUniform("u_kaleidoSlices", m_data.slices.first);
    m_shader.setUniform("u_swirlStrength", m_data.swirlStrength.first);
    m_shader.setUniform("u_swirlDensity", m_data.swirlDensity.first);
    m_shader.setUniform("u_angularPulseFreq", m_data.pulseFrequency.first);
    m_shader.setUniform("u_pulseStrength", m_data.pulseStrength.first);
    m_shader.setUniform("u_pulseSpeed", m_data.pulseSpeed.first);
    m_shader.setUniform("u_angleSteps", m_data.angleSteps.first);
    m_shader.setUniform("u_radialStretch", m_data.radialStretch.first);
    m_shader.setUniform("u_noiseStrength", m_data.noiseStrength.first);

    m_outputTexture.clear( sf::Color::Transparent );
    m_outputTexture.draw( sf::Sprite( inputTexture->getTexture() ), &m_shader );
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                                  m_outputTexture.get(),
                                  m_data.mixFactor.first );
  }

}