#include "models/shader/DualKawaseBlurShader.hpp"

#include "helpers/CommonHeaders.hpp"

namespace nx
{

  DualKawaseBlurShader::DualKawaseBlurShader( PipelineContext& context )
    : m_ctx( context )
  {
    if ( !m_shader.loadFromMemory( m_fragmentShader, sf::Shader::Type::Fragment ) ||
         !m_compositeShader.loadFromMemory( m_compositeFragmentShader, sf::Shader::Type::Fragment ) )
    {
      LOG_ERROR( "Failed to load dk blur fragment shader" );
    }
    else
    {
      LOG_INFO( "loaded dk blur shader" );
    }

    EXPAND_SHADER_VST_BINDINGS(DUALKAWASE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  DualKawaseBlurShader::~DualKawaseBlurShader()
  {
    m_ctx.vstContext.paramBindingManager.unregisterAllControlsOwnedBy( this );
  }

  ///////////////////////////////////////////////////////
  /// ISERIALIZABLE
  ///////////////////////////////////////////////////////

  nlohmann::json DualKawaseBlurShader::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum(getType());
    EXPAND_SHADER_PARAMS_TO_JSON(DUALKAWASE_SHADER_PARAMS)
    j[ "midiTriggers" ] = m_midiNoteControl.serialize();
    j[ "easing" ] = m_easing.serialize();
    return j;
  }

  void DualKawaseBlurShader::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(DUALKAWASE_SHADER_PARAMS)

      m_midiNoteControl.deserialize( j[ "midiTriggers" ] );
      m_easing.deserialize( j[ "easing" ] );
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }

    EXPAND_SHADER_VST_BINDINGS(DUALKAWASE_SHADER_PARAMS, m_ctx.vstContext.paramBindingManager)
  }

  void DualKawaseBlurShader::drawMenu()
  {
    if ( ImGui::TreeNode( "Dual-Kawase Blur Options" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(DUALKAWASE_SHADER_PARAMS, m_data)

      ImGui::Separator();
      m_easing.drawMenu();

      ImGui::Separator();
      m_midiNoteControl.drawMenu();

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  void DualKawaseBlurShader::trigger( const Midi_t& midi )
  {
    if ( m_midiNoteControl.empty() || m_midiNoteControl.isNoteActive( midi.pitch ) )
      m_easing.trigger();
  }

  [[nodiscard]]
  bool DualKawaseBlurShader::isShaderActive() const  { return m_data.isActive; }

  [[nodiscard]]
  sf::RenderTexture * DualKawaseBlurShader::applyShader(const sf::RenderTexture * inputTexture)
  {
    m_pingTexture.ensureSize( inputTexture->getSize() );
    m_pongTexture.ensureSize( inputTexture->getSize() );
    m_compositeTexture.ensureSize( inputTexture->getSize() );

    sf::RenderTexture* src = m_pingTexture.get();
    sf::RenderTexture* dst = m_pongTexture.get();

    m_pingTexture.clear();
    m_pingTexture.draw(sf::Sprite(inputTexture->getTexture()));
    m_pingTexture.display();

    const auto easing = m_easing.getEasing();

    for (int i = 0; i < m_data.passes.first; ++i)
    {
      dst->clear();

      m_shader.setUniform("u_texture", src->getTexture());
      m_shader.setUniform("u_texelSize", sf::Glsl::Vec2(1.f / static_cast< float >(inputTexture->getSize().x),
                                                 1.f / static_cast< float >(inputTexture->getSize().y)));
      m_shader.setUniform("u_offset", m_data.offset.first + static_cast< float >(i)); // optional increase per pass
      m_shader.setUniform("u_bloomGain", m_data.bloomGain.first * easing);     // user/MIDI-driven
      m_shader.setUniform("u_brightness", m_data.brightness.first * easing);   // compensate blur

      dst->draw(sf::Sprite(src->getTexture()), &m_shader);
      dst->display();

      std::swap(src, dst); // ping-pong
    }

    // Composite final bloom with original
    m_compositeTexture.clear();
    m_compositeShader.setUniform("u_scene", inputTexture->getTexture());
    m_compositeShader.setUniform("u_bloom", src->getTexture());
    m_compositeShader.setUniform("u_mixFactor", m_data.mixFactor.first * easing);

    m_compositeTexture.draw( sf::Sprite( inputTexture->getTexture() ), &m_compositeShader );
    m_compositeTexture.display();

    return m_compositeTexture.get();
  }

}