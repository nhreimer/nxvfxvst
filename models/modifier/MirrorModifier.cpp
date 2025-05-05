#include "models/modifier/MirrorModifier.hpp"

namespace nx
{
  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////
  nlohmann::json MirrorModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(MIRROR_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void MirrorModifier::deserialize( const nlohmann::json& j )
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(MIRROR_MODIFIER_PARAMS)
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
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(MIRROR_MODIFIER_PARAMS, m_data)

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
      const float angleOffsetRad = m_data.angleOffsetDegrees.first * NX_D2R;

      const float radius = ( m_data.useDynamicRadius.first )
        ? m_data.distance.first * m_data.dynamicRadius.first * m_data.lastVelocityNorm.first
        : m_data.distance.first;

      for (int i = 0; i < m_data.count.first; ++i)
      {
        const float angle = baseAngle + angleOffsetRad + i * (2.f * NX_PI / m_data.count.first);
        const sf::Vector2f mirrorPos =
        {
          origin.x + std::cos(angle) * radius,
          origin.y + std::sin(angle) * radius
        };

        auto* shape = new sf::CircleShape(p->shape); // copy
        auto fillColor = m_data.useParticleColors.first
          ? shape->getFillColor()
          : m_data.mirrorColor.first;

        fillColor.a = static_cast< uint8_t >(fillColor.a * m_data.mirrorAlpha.first);
        shape->setFillColor(fillColor);

        if ( shape->getOutlineThickness() > 0.f )
        {
          auto outlineColor = m_data.useParticleColors.first
            ? shape->getOutlineColor()
            : m_data.mirrorOutlineColor.first;

          outlineColor.a = static_cast< uint8_t >(outlineColor.a * m_data.mirrorAlpha.first);
          shape->setOutlineColor(outlineColor);
        }

        shape->setPosition(mirrorPos);
        outArtifacts.push_back(shape); // user doesn't own memory
      }
    }
  }
} // namespace nx
