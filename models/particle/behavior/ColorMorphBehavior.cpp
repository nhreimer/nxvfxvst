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
    return
 {
    { "type", SerialHelper::serializeEnum( getType() ) },
      { "speed", m_data.speed },
      { "saturation", m_data.saturation },
      { "brightness", m_data.brightness },
      { "quantizeStep", m_data.quantizeStep },
      { "useHueOffset", m_data.useHueOffset },
      { "useSkittles", m_data.useSkittles },
      { "reverseColors", m_data.reverseColors },
      { "morphDuration", m_data.morphDuration },
      { "morphToColor", SerialHelper::convertColorToJson( m_data.morphToColor ) }
    };
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::deserialize(const nlohmann::json &j)
  {
    m_data.speed = j.at( "speed" ).get<float>();
    m_data.saturation = j.at( "saturation" ).get<float>();
    m_data.brightness = j.at( "brightness" ).get<float>();
    m_data.quantizeStep = j.at( "quantizeStep" ).get<float>();
    m_data.useHueOffset = j.at( "useHueOffset" ).get<bool>();
    m_data.useSkittles = j.at( "useSkittles" ).get<bool>();
    m_data.reverseColors = j.at( "reverseColors" ).get<bool>();
    m_data.morphToColor = SerialHelper::convertColorFromJson( j.at( "morphToColor" ) );
    m_data.morphDuration = j.at( "morphDuration" ).get<float>();
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::applyOnSpawn( TimedParticle_t * p,
                                         const Midi_t& midi )
  {
    // Optionally inject random hue offset based on pitch
    m_data.hueOffset = (m_data.useHueOffset)
      ? static_cast<float>(midi.pitch % 128) / 127.f * 360.f
      : 0.f;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void ColorMorphBehavior::applyOnUpdate( TimedParticle_t * p,
                                          const sf::Time& deltaTime )
  {
    if ( m_data.useSkittles )
      applySkittlesMorphing( p, deltaTime );
    else if ( m_data.reverseColors )
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
      ImGui::Checkbox( "Use Hue Offset", &m_data.useHueOffset );
      ImGui::Checkbox( "Use Skittles Morph", &m_data.useSkittles );
      ImGui::SliderFloat("Hue Quantize Step", &m_data.quantizeStep, 0.f, 90.f );
      ImGui::SliderFloat("Saturation", &m_data.saturation, 0.f, 1.f);
      ImGui::SliderFloat("Brightness", &m_data.brightness, 0.f, 1.f);
      ImGui::SliderFloat("Color Morph Speed", &m_data.speed, 0.f, 5.f);

      ImGui::Checkbox( "Reverse Bounce", &m_data.reverseColors );
      ImGui::SliderFloat("Morph Duration (sec)", &m_data.morphDuration, 0.1f, 10.f);
      ImVec4 color = m_data.morphToColor;

      if ( ImGui::ColorEdit4("Target Color", reinterpret_cast< float * >( &color) ) )
        m_data.morphToColor = color;

      ImGui::TreePop();
      ImGui::Separator();
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  void ColorMorphBehavior::applyBicolorMorphing( TimedParticle_t * p,
                                                 const sf::Time& deltaTime ) const
  {
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;
    const float t = elapsed / m_data.morphDuration;

    const auto morphed = ColorHelper::lerpColor(p->initialColor, m_data.morphToColor, t);
    p->shape.setFillColor(morphed);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  void ColorMorphBehavior::applyBicolorMorphingReverse( TimedParticle_t * p,
                                                        const sf::Time& deltaTime ) const
  {
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;

    const float t = 0.5f * (1.f + std::sin(elapsed * m_data.speed * NX_TAU));
    const auto morphed = ColorHelper::lerpColor( p->initialColor, m_data.morphToColor, t );
    p->shape.setFillColor(morphed);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  void ColorMorphBehavior::applySkittlesMorphing( TimedParticle_t * p,
                                                  const sf::Time& deltaTime ) const
  {
    // How long the particle has existed
    const float elapsed = m_ctx.globalInfo.elapsedTimeSeconds - p->spawnTime;

    // Base hue animation based on time and speed
    const float hueBase = std::fmod(elapsed * m_data.speed * 360.f, 360.f);

    // Final hue is base + offset
    float finalHue = std::fmod(hueBase + m_data.hueOffset, 360.f);

    // Optionally quantize the hue
    if (m_data.quantizeStep > 0.f)
      finalHue = std::floor(finalHue / m_data.quantizeStep) * m_data.quantizeStep;

    // Preserve alpha from original color
    const auto morphed = hsvToRgb(finalHue, m_data.saturation, m_data.brightness, p->initialColor.a);

    // Apply the new color
    p->shape.setFillColor(morphed);
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  sf::Color ColorMorphBehavior::hsvToRgb(const float h,
                                         const float s,
                                         const float v,
                                         const uint8_t alpha)
  {
    float c = v * s;
    float x = c * (1 - std::fabs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;

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