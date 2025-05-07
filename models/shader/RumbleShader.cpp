#include "models/shader/RumbleShader.hpp"

namespace nx
{

  RumbleShader::RumbleShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load rumble fragment shader" );
    }
    else
    {
      LOG_DEBUG( "Rumble fragment shader loaded successfully" );
    }

    EXPAND_SHADER_VST_BINDINGS(RUMBLE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  RumbleShader::~RumbleShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  [[nodiscard]]
  nlohmann::json RumbleShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(RUMBLE_SHADER_PARAMS)

    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void RumbleShader::deserialize( const nlohmann::json &j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(RUMBLE_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void RumbleShader::drawMenu()
  {
    if ( ImGui::TreeNode("Rumble Options" ) )
    {
      ImGui::Checkbox("Active", &m_data.isActive);
      EXPAND_SHADER_IMGUI(RUMBLE_SHADER_PARAMS, m_data)

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void RumbleShader::trigger( const Midi_t &midi )
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  [[nodiscard]]
  bool RumbleShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * RumbleShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    const float time = m_clock.getElapsedTime().asSeconds();
    const float pulse = m_easing.getEasing();

    m_shader.setUniform("texture", inputTexture->getTexture());
    m_shader.setUniform("resolution", sf::Vector2f(inputTexture->getSize()));
    m_shader.setUniform("time", time);

    m_shader.setUniform("rumbleStrength", m_data.rumbleStrength.first);
    m_shader.setUniform("frequency", m_data.frequency.first);
    m_shader.setUniform("pulseValue", pulse); // now driven by easing
    m_shader.setUniform("direction", sf::Glsl::Vec2(m_data.direction.first));
    m_shader.setUniform("useNoise", m_data.useNoise.first);

    m_shader.setUniform("modAmplitude", m_data.modAmplitude.first);
    m_shader.setUniform("modFrequency", m_data.modFrequency.first);
    m_shader.setUniform("colorDesync", pulse * m_data.maxColorDesync.first + m_data.baseColorDesync.first);
    //m_shader.setUniform("colorDesync", m_data.colorDesync);

    m_outputTexture.clear();
    m_outputTexture.draw(sf::Sprite(inputTexture->getTexture()), &m_shader);
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                                  m_outputTexture.get(),
                                  m_data.mixFactor.first );
  }

}