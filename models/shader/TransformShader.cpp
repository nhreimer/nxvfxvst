#include "models/shader/TransformShader.hpp"

namespace nx
{
  TransformShader::TransformShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load transform fragment shader" );
    }
    else
    {
      LOG_DEBUG( "Transform fragment shader loaded successfully" );
      m_easing.setEasingType( E_EasingType::E_Disabled );
    }

    EXPAND_SHADER_VST_BINDINGS(TRANSFORM_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  TransformShader::~TransformShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  [[nodiscard]]
  nlohmann::json TransformShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(TRANSFORM_SHADER_PARAMS)

    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void TransformShader::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(TRANSFORM_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void TransformShader::trigger(const Midi_t &midi)
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  void TransformShader::drawMenu()
  {
    if ( ImGui::TreeNode("Transform Options") )
    {

      const float offsetX = m_data.shift.first.x;
      const float offsetY = m_data.shift.first.y;

      EXPAND_SHADER_IMGUI(TRANSFORM_SHADER_PARAMS, m_data)

      if ( offsetX != m_data.shift.first.x || offsetY != m_data.shift.first.y )
      {
        const sf::Vector2f calibrated
        {
          ( m_data.shift.first.x + 0.5f ) * static_cast< float >( m_ctx.globalInfo.windowSize.x ),
          ( m_data.shift.first.y + 0.5f ) * static_cast< float >( m_ctx.globalInfo.windowSize.y )
        };

        m_timedCursorShift.setPosition( calibrated );
      }

      ImGui::SeparatorText( "Easings" );
      m_easing.drawMenu();

      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_timedCursorShift.hasExpired() )
      m_timedCursorShift.drawPosition();
  }

  [[nodiscard]]
  bool TransformShader::isShaderActive() const { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * TransformShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );

    auto const easing = m_easing.getEasing();

    m_shader.setUniform("u_texture", inputTexture->getTexture());
    m_shader.setUniform("u_resolution", sf::Vector2f { inputTexture->getSize() });
    m_shader.setUniform("u_offset", sf::Glsl::Vec2( m_data.shift.first ) );
    m_shader.setUniform("u_scale", m_data.scale.first * easing );

    m_shader.setUniform("u_rotation", sf::degrees(m_data.rotationDegrees.first).asRadians());
    m_shader.setUniform("u_flipX", m_data.flipX.first);
    m_shader.setUniform("u_flipY", m_data.flipY.first);

    m_outputTexture.clear();
    m_outputTexture.draw(sf::Sprite(inputTexture->getTexture()), &m_shader);
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                            m_outputTexture.get(),
                            m_data.mixFactor.first );
  }

}