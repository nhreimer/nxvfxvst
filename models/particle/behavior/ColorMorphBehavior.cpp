#include "models/particle/behavior/ColorMorphBehavior.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  [[nodiscard]]
  nlohmann::json ColorMorphBehavior::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(COLOR_MORPH_BEHAVIOR_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(COLOR_MORPH_BEHAVIOR_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::applyOnSpawn( IParticle * p,
                       const Midi_t& midiEvent,
                       const ParticleData_t& particleData,
                       const sf::Vector2f& position )
  {
    // Optionally inject random hue offset based on pitch
    m_data.hueOffset.first = (m_data.useHueOffset.first)
      ? static_cast<float>(midiEvent.pitch % 128) / 127.f * 360.f
      : 0.f;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::applyOnUpdate( IParticle * p,
                        const sf::Time& deltaTime,
                        const ParticleData_t& particleData,
                        const sf::Vector2f& position )
  {
    if ( m_data.useSkittles.first )
      applySkittlesMorphing( p, deltaTime );
    else if ( m_data.reverseColors.first )
      applyBicolorMorphingReverse( p, deltaTime );
    else
      applyBicolorMorphing( p, deltaTime );
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::drawMenu()
  {
    if ( ImGui::TreeNode( "Color Morph Behavior" ) )
    {
      EXPAND_SHADER_IMGUI(COLOR_MORPH_BEHAVIOR_PARAMS, m_data)
      ImGui::TreePop();
      ImGui::Separator();
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  void ColorMorphBehavior::applyBicolorMorphing( IParticle * p,
                                                 const sf::Time& deltaTime ) const
  {
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->getSpawnTimeInSeconds();
    const float t = elapsed / m_data.morphDuration.first;
    const auto colors = p->getColors();
    const auto morphedStart = ColorHelper::lerpColor( colors.first, m_data.morphToColor.first, t);
    const auto morphedEnd = ColorHelper::lerpColor(colors.second, m_data.morphToColor.first, t);
    p->setColorPattern(morphedStart, morphedEnd);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  void ColorMorphBehavior::applyBicolorMorphingReverse( IParticle * p,
                                                        const sf::Time& deltaTime ) const
  {
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->getSpawnTimeInSeconds();

    const float t = 0.5f * (1.f + std::sin(elapsed * m_data.speed.first * NX_TAU));
    const auto colors = p->getColors();
    const auto morphedStart = ColorHelper::lerpColor(colors.first, m_data.morphToColor.first, t);
    const auto morphedEnd = ColorHelper::lerpColor(colors.second, m_data.morphToColor.first, t);
    p->setColorPattern(morphedStart, morphedEnd);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  void ColorMorphBehavior::applySkittlesMorphing( IParticle * p,
                                                  const sf::Time& deltaTime ) const
  {
    // How long the particle has existed
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->getSpawnTimeInSeconds();

    // Base hue animation based on time and speed
    const float hueBase = std::fmod(elapsed * m_data.speed.first * 360.f, 360.f);

    // Final hue is base + offset
    float finalHue = std::fmod(hueBase + m_data.hueOffset.first, 360.f);

    // Optionally quantize the hue
    if (m_data.quantizeStep.first > 0.f)
      finalHue = std::floor(finalHue / m_data.quantizeStep.first) * m_data.quantizeStep.first;

    // Preserve alpha from original color
    const auto colors = p->getColors();
    const auto morphedStart = hsvToRgb(finalHue, m_data.saturation.first, m_data.brightness.first, colors.first.a);
    const auto morphedEnd = hsvToRgb(finalHue, m_data.saturation.first, m_data.brightness.first, colors.second.a);

    // Apply the new color
    p->setColorPattern(morphedStart, morphedEnd);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  sf::Color ColorMorphBehavior::hsvToRgb(const float h,
                                         const float s,
                                         const float v,
                                         const uint8_t alpha)
  {
    const float c = v * s;
    const float x = c * (1 - std::fabs(std::fmod(h / 60.0f, 2) - 1));
    const float m = v - c;

    float r, g, b;
    if      (h < 60)  { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    return sf::Color(
      static_cast<uint8_t>((r + m) * 255),
      static_cast<uint8_t>((g + m) * 255),
      static_cast<uint8_t>((b + m) * 255),
      alpha
    );
  }

}