#include "models/shader/BlurShader.hpp"

namespace nx
{

  BlurShader::BlurShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory(m_fragmentShader, sf::Shader::Type::Fragment) )
    {
      LOG_ERROR("Failed to load blur fragment shader");
    }
    else
    {
      LOG_INFO("loaded blur shader");
    }

    EXPAND_SHADER_VST_BINDINGS(BLUR_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  BlurShader::~BlurShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json BlurShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(BLUR_SHADER_PARAMS)

    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void BlurShader::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(BLUR_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  void BlurShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Gaussian Blur Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );

      EXPAND_SHADER_IMGUI(BLUR_SHADER_PARAMS, m_data)

      ImGui::SeparatorText( "Easings" );
      m_easing.drawMenu();

      ImGui::SeparatorText( "Midi Triggers" );
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void BlurShader::trigger( const Midi_t& midi )
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  [[nodiscard]]
  bool BlurShader::isShaderActive() const
  {
    return m_data.isActive;
  }

  [[nodiscard]]
  sf::RenderTexture * BlurShader::applyShader(
    const sf::RenderTexture * inputTexture )
  {
    m_outputTexture.ensureSize( inputTexture->getSize() );
    m_intermediary.ensureSize( inputTexture->getSize() );

    const float easing = m_easing.getEasing();

    const sf::Sprite sprite( inputTexture->getTexture() );

    // Apply horizontal blur
    m_shader.setUniform( "texture", inputTexture->getTexture() );
    m_shader.setUniform( "direction", sf::Glsl::Vec2( 1.f, 0.f ) ); // Horizontal
    m_shader.setUniform( "blurRadiusX", m_data.blurHorizontal.first );
    m_shader.setUniform( "blurRadiusY", 0.f ); // No vertical blur in this pass
    m_shader.setUniform( "sigma", m_data.sigma.first );
    m_shader.setUniform( "brighten", m_data.brighten.first );

    m_shader.setUniform( "intensity", easing );

    m_intermediary.clear();
    m_intermediary.draw(sprite, &m_shader);
    m_intermediary.display();

    // Apply vertical blur
    m_shader.setUniform("texture", m_intermediary.getTexture());
    m_shader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f)); // Vertical
    m_shader.setUniform("blurRadiusX", 0.f); // No horizontal blur in this pass
    m_shader.setUniform("blurRadiusY", m_data.blurVertical.first);
    m_shader.setUniform( "sigma", m_data.sigma.first );
    m_shader.setUniform( "brighten", m_data.brighten.first );
    m_shader.setUniform( "intensity",easing );

    m_outputTexture.clear(sf::Color::Transparent);
    m_outputTexture.draw(sprite, &m_shader);
    m_outputTexture.display();

    return m_blender.applyShader( inputTexture,
                                  m_outputTexture.get(),
                                  m_data.mixFactor.first );
  }


}