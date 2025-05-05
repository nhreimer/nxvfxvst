#include "models/modifier/PerlinDeformerModifier.hpp"

namespace nx
{

  /////////////////////////////////////////////////////////
  /// PUBLIC
  /////////////////////////////////////////////////////////

  [[nodiscard]]
  nlohmann::json PerlinDeformerModifier::serialize() const
  {
    nlohmann::json j;
    j[ "type" ] = SerialHelper::serializeEnum( getType() );
    EXPAND_SHADER_PARAMS_TO_JSON(PERLIN_DEFORMER_MODIFIER_PARAMS)
    return j;
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::deserialize(const nlohmann::json &j)
  {
    if ( SerialHelper::isTypeGood( j, getType() ) )
    {
      EXPAND_SHADER_PARAMS_FROM_JSON(PERLIN_DEFORMER_MODIFIER_PARAMS)
    }
    else
    {
      LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::drawMenu()
  {
    if ( ImGui::TreeNode( "Perlin Deformer" ) )
    {
      ImGui::Checkbox( "Is Active##1", &m_data.isActive );
      EXPAND_SHADER_IMGUI(PERLIN_DEFORMER_MODIFIER_PARAMS, m_data)

      ImGui::SeparatorText( "Perlin Deformer Types" );
      if ( ImGui::RadioButton( "Hash##1", m_data.noiseType == E_NoiseType::E_Hash ) )
      {
        m_data.noiseType = E_NoiseType::E_Hash;
      }
      else if ( ImGui::RadioButton( "Value##1", m_data.noiseType == E_NoiseType::E_Value ) )
      {
        m_data.noiseType = E_NoiseType::E_Value;
      }
      else if ( ImGui::RadioButton( "FBM##1", m_data.noiseType == E_NoiseType::E_FBM ) )
      {
        m_data.noiseType = E_NoiseType::E_FBM;
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }
  }

  /////////////////////////////////////////////////////////
  /// PUBLIC
  void PerlinDeformerModifier::modify(
     const ParticleLayoutData_t& particleLayoutData,
     std::deque< TimedParticle_t* >& particles,
     std::deque< sf::Drawable* >& outArtifacts )
  {
    for (size_t i = 0; i < particles.size(); ++i)
    {
      const sf::Vector2f pos = particles[ i ]->shape.getPosition();
      const float x = pos.x * m_data.noiseScale.first;
      const float y = pos.y * m_data.noiseScale.first;

      const float offsetX = (getNoise(x + m_time, y) - 0.5f) * 2.f * m_data.deformStrength.first;
      const float offsetY = (getNoise(x, y + m_time) - 0.5f) * 2.f * m_data.deformStrength.first;

      const sf::Vector2f warpedPos = pos + sf::Vector2f(offsetX, offsetY);

      // don't get stuck in an infinite loop
      //auto *copiedParticle = particles.emplace_back(new TimedParticle_t(*particles[ i ]));
      //copiedParticle->shape.setPosition(warpedPos);
      auto * copiedShape = dynamic_cast< sf::CircleShape * >(
        outArtifacts.emplace_back( new sf::CircleShape( particles[ i ]->shape ) ) );

      copiedShape->setPosition( warpedPos );
      const auto color = ( m_data.useParticleColors.first )
        ? copiedShape->getFillColor()
        : m_data.perlinColor.first;

      copiedShape->setFillColor( { color.r, color.g, color.b, static_cast< uint8_t >(color.a * m_data.colorFade.first) } );
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE
  /////////////////////////////////////////////////////////
  float PerlinDeformerModifier::getNoise( const float x, const float y ) const
  {
    switch ( m_data.noiseType )
    {
      case E_NoiseType::E_Hash:  return getHashNoise(x, y);
      case E_NoiseType::E_Value: return getValueNoise(x, y);
      case E_NoiseType::E_FBM:   return getFBM(x, y, m_data.octaves.first);
      default: return 0.f;
    }
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // hash noise: fast but low quality
  float PerlinDeformerModifier::getHashNoise(float x, float y)
  {
    const float dotVal = x * 12.9898f + y * 78.233f;
    const float sinVal = std::sin(dotVal) * 43758.5453f;
    return sinVal - std::floor(sinVal); // now in [0.0, 1.0)
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // Smooth blend between grid points
  // Simple 2D value noise with bilinear interpolation
  float PerlinDeformerModifier::getValueNoise( const float x, const float y)
  {
    const int xi = static_cast<int>(std::floor(x));
    const int yi = static_cast<int>(std::floor(y));
    const float xf = x - xi;
    const float yf = y - yi;

    const float tl = getHashNoise(xi, yi);
    const float tr = getHashNoise(xi + 1, yi);
    const float bl = getHashNoise(xi, yi + 1);
    const float br = getHashNoise(xi + 1, yi + 1);

    const float top = lerp(tl, tr, xf);
    const float bottom = lerp(bl, br, xf);

    return lerp(top, bottom, yf); // [0,1]
  }

  /////////////////////////////////////////////////////////
  /// PRIVATE STATIC
  // Fractal Brownian Motion
  // Layered noise — adds complexity and control
  float PerlinDeformerModifier::getFBM( const float x, const float y, const int octaves)
  {
    float value = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;

    for (int i = 0; i < octaves; ++i)
    {
      value += amplitude * getValueNoise(x * frequency, y * frequency);
      frequency *= 2.0f;
      amplitude *= 0.5f;
    }

    return value;
  }

}