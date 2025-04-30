#include "models/modifier/MirrorModifier.hpp"

#include "helpers/ColorHelper.hpp"

namespace nx
{
  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json MirrorModifier::serialize() const
  {
    return
    {
      { "type", getType() },
      { "isActive", m_data.isActive },
      { "count", m_data.count },
      { "distance", m_data.distance },
      { "mirrorAlpha", m_data.mirrorAlpha },
      { "lastVelocityNorm", m_data.lastVelocityNorm },
      { "useDynamicRadius", m_data.useDynamicRadius },
      { "dynamicRadius", m_data.dynamicRadius },
      { "angleOffsetDegrees", m_data.angleOffsetDegrees }
    };
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      m_data.isActive = j.at( "isActive" ).get<bool>();
      m_data.count = j.at( "count" ).get<int>();
      m_data.distance = j.at( "distance" ).get<float>();
      m_data.mirrorAlpha = j.at( "mirrorAlpha" ).get<float>();
      m_data.lastVelocityNorm = j.at( "lastVelocityNorm" ).get<float>();
      m_data.useDynamicRadius = j.at( "useDynamicRadius" ).get<bool>();
      m_data.dynamicRadius = j.at( "dynamicRadius" ).get<float>();
      m_data.angleOffsetDegrees = j.at( "angleOffsetDegrees" ).get<float>();
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Mirror Modifier" ) )
    {
      ImGui::Checkbox( "Mirror Enabled", &m_data.isActive );
      ImGui::SliderInt("Mirror Count", &m_data.count, 1, 16);
      ImGui::SliderFloat("Mirror Distance", &m_data.distance, 10.f, 300.f);
      ImGui::SliderFloat("Mirror Opacity", &m_data.mirrorAlpha, 0.f, 1.f);
      ImGui::SliderFloat("Mirror Rotation (deg)", &m_data.angleOffsetDegrees, 0.f, 360.f);

      ImGui::Checkbox( "Velocity Radius", &m_data.useDynamicRadius );
      ImGui::SliderFloat( "Mirror Velocity Amplifier", &m_data.dynamicRadius, 0.f, 5.f);

      ImGui::Checkbox( "Use Particle Colors", &m_data.useParticleColors );
      if ( !m_data.useParticleColors )
      {
        ColorHelper::drawImGuiColorEdit4( "Mirrored Color", m_data.mirrorColor );
        ColorHelper::drawImGuiColorEdit4( "Mirrored Outline Color", m_data.mirrorOutlineColor );
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::modify(const ParticleLayoutData_t & layoutData,
              std::deque< TimedParticle_t * > &particles,
              std::deque< sf::Drawable * > &outArtifacts)
  {
    for (const auto* p : particles)
    {
      const sf::Vector2f origin = p->shape.getPosition();
      const float baseAngle =
        std::atan2(origin.y - m_ctx.globalInfo.windowHalfSize.y, origin.x - m_ctx.globalInfo.windowHalfSize.x);
      const float angleOffsetRad = m_data.angleOffsetDegrees * NX_D2R;

      const float radius = ( m_data.useDynamicRadius )
        ? m_data.distance * m_data.dynamicRadius * m_data.lastVelocityNorm
        : m_data.distance;

      for (int i = 0; i < m_data.count; ++i)
      {
        const float angle = baseAngle + angleOffsetRad + i * (2.f * NX_PI / m_data.count);
        const sf::Vector2f mirrorPos =
        {
          origin.x + std::cos(angle) * radius,
          origin.y + std::sin(angle) * radius
        };

        auto* shape = new sf::CircleShape(p->shape); // copy
        auto fillColor = m_data.useParticleColors
          ? shape->getFillColor()
          : m_data.mirrorColor;

        fillColor.a = static_cast< uint8_t >(fillColor.a * m_data.mirrorAlpha);
        shape->setFillColor(fillColor);

        if ( shape->getOutlineThickness() > 0.f )
        {
          auto outlineColor = m_data.useParticleColors
            ? shape->getOutlineColor()
            : m_data.mirrorOutlineColor;

          outlineColor.a = static_cast< uint8_t >(outlineColor.a * m_data.mirrorAlpha);
          shape->setOutlineColor(outlineColor);
        }

        shape->setPosition(mirrorPos);
        outArtifacts.push_back(shape); // user doesn't own memory
      }
    }
  }
} // namespace nx
