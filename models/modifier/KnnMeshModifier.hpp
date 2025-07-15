#pragma once

#include "helpers/LineHelper.hpp"

namespace nx
{

  class KnnMeshModifier final : public IParticleModifier
  {

#define KNN_MESH_LINE_MODIFIER_PARAMS(X)                                                                   \
X(kNeighbors,        int32_t,   3,      1,      10,      "The number of neighbors",               true)    \
X(maxDistance,       float,     150.f,  10.f,   500.f,   "Max distance when considering neighbor", true)   \
X(lineThickness,     float,     2.0f,   0.1f,   100.0f,  "Thickness of the curved line",          true)    \
X(swellFactor,       float,     1.5f,   0.0f,   10.0f,   "Swelling multiplier at midpoint",       true)    \
X(easeDownInSeconds, float,     1.0f,   0.01f,  10.0f,   "Ease-out fade duration (seconds)",      true)    \
X(useParticleColors, bool,      true,   0,      1,       "Use original particle colors",          true)    \
X(lineColor,         sf::Color, sf::Color(255,255,255,255), 0, 255, "Primary fallback line color", false)  \
X(otherLineColor,    sf::Color, sf::Color(255,255,255,255), 0, 255, "Alternate/fading line color", false)  \
X(invertColorTime,   bool,      false,  0,      1,       "Colors fade in over time rather than out", true) \
X(curvature,         float,     0.25f,  -NX_PI,  NX_PI,  "Amount of curvature (arc)",             true)    \
X(lineSegments,      int32_t,   20,     1,      200,     "Number of segments in the curve",       true)
// X(coneAngleDeg,      float,    45.f, 5.f, 180.f, "Max cone angle", true)                                   \
// X(maxConnections,    int32_t,  2, 1, 8, "Connections per particle", true)                                  \
// X(direction,         sf::Vector2f, sf::Vector2f(0.f, 0.f), 0.f, 0.f, "Bias direction", false)

    struct KnnMeshData_t
    {
      bool isActive { true };
      EXPAND_SHADER_PARAMS_FOR_STRUCT(KNN_MESH_LINE_MODIFIER_PARAMS)
    };

    enum class E_KnnMeshLineModifierParam
    {
      EXPAND_SHADER_PARAMS_FOR_ENUM(KNN_MESH_LINE_MODIFIER_PARAMS)
      LastItem
    };

    static inline const std::array<std::string, static_cast<size_t>(E_KnnMeshLineModifierParam::LastItem)> m_paramLabels =
    {
      EXPAND_SHADER_PARAM_LABELS(KNN_MESH_LINE_MODIFIER_PARAMS)
    };

  public:

    explicit KnnMeshModifier( PipelineContext& ctx )
      : m_ctx( ctx )
    {
      EXPAND_SHADER_VST_BINDINGS(KNN_MESH_LINE_MODIFIER_PARAMS, m_ctx.vstContext.paramBindingManager)
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      nlohmann::json j;
      j[ "type" ] = SerialHelper::serializeEnum( getType() );
      EXPAND_SHADER_PARAMS_TO_JSON(KNN_MESH_LINE_MODIFIER_PARAMS)
      return j;
    }

    void deserialize(const nlohmann::json &j) override
    {
      if ( SerialHelper::isTypeGood( j, getType() ) )
      {
        EXPAND_SHADER_PARAMS_FROM_JSON(KNN_MESH_LINE_MODIFIER_PARAMS)
      }
      else
      {
        LOG_DEBUG( "failed to find type for {}", SerialHelper::serializeEnum( getType() ) );
      }
    }

    [[nodiscard]]
    E_ModifierType getType() const override { return E_ModifierType::E_KnnMeshModifier; }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "KNN Mesh" ) )
      {
        EXPAND_SHADER_IMGUI(KNN_MESH_LINE_MODIFIER_PARAMS, m_data)

        ImGui::TreePop();
        ImGui::Spacing();
      }

    }

    void update(const sf::Time&) override {}

    bool isActive() const override { return m_data.isActive; }

    void processMidiEvent(const Midi_t&) override {}

    void modify(const sf::BlendMode& blendMode,
                std::deque<IParticle*>& particles,
                std::deque<sf::Drawable*>& outArtifacts) override
    {
      if (particles.size() < 3)
        return;

      const size_t count = particles.size();

      for (size_t i = 0; i < count; ++i)
      {
        const auto* a = particles[i];
        const auto& posA = a->getPosition();

        std::vector< std::pair< float, size_t > > distances;
        for (size_t j = 0; j < count; ++j)
        {
          if (i == j) continue;

          const auto* b = particles[j];
          const float dist = MathHelper::getDistance(posA, b->getPosition());
          if (dist <= m_data.maxDistance.first)
            distances.emplace_back(dist, j);
        }

        std::ranges::sort(distances);

        const size_t limit = std::min<size_t>(m_data.kNeighbors.first, distances.size());
        for (size_t k = 0; k < limit; ++k)
        {
          const auto& [dist, j] = distances[k];
          const auto* b = particles[j];

          auto * line = new CurvedLine(
            posA,
            b->getPosition(),
            m_data.curvature.first,
            m_data.lineSegments.first );

          outArtifacts.emplace_back( line );

          line->setWidth( m_data.lineThickness.first );

          if ( m_data.useParticleColors.first )
          {
            LineHelper::updateLineColors( line,
              a,
              b,
              m_data.invertColorTime.first );
          }
          else
          {
            LineHelper::updateCustomLineColors(
              line,
              b,
              a,
              m_data.lineColor.first,
              m_data.otherLineColor.first,
              m_data.invertColorTime.first );
          }
        }
      }
    }

    // this uses a few directional bias options that are experimental
    // void modify(
    //   const ParticleLayoutData_t& layoutData,
    //   std::deque<IParticle*>& particles,
    //   std::deque<sf::Drawable*>& outArtifacts) override
    // {
    //   if (!isActive() || particles.empty()) return;
    //
    //   const float maxAngle = m_data.coneAngleDeg.first * 0.5f;
    //   const float maxAngleRad = maxAngle * (NX_PI / 180.f);
    //
    //   const sf::Vector2f normalizedDir = normalize(m_data.direction.first);
    //
    //   for (const auto * source : particles)
    //   {
    //     std::vector< std::pair< IParticle*, float > > candidates;
    //
    //     for (auto * target : particles)
    //     {
    //       if (source == target)
    //         continue;
    //
    //       const sf::Vector2f toTarget = target->getPosition() - source->getPosition();
    //       const float dist = length(toTarget);
    //       const sf::Vector2f dir = ( dist == 0.0f )
    //         ? sf::Vector2f { 0.f, 0.f }
    //         : toTarget / dist;
    //
    //       const float angle = std::acos(std::clamp(dot(dir, normalizedDir), -1.f, 1.f));
    //
    //       if (angle <= maxAngleRad)
    //         candidates.emplace_back(target, dist);
    //     }
    //
    //     std::ranges::sort(candidates, [](const auto& a, const auto& b)
    //     {
    //       return a.second < b.second;
    //     });
    //
    //     const auto minSize = std::min<int32_t>(m_data.maxConnections.first, candidates.size());
    //
    //     for (int32_t i = 0; i < minSize; ++i)
    //     {
    //       auto * line = new CurvedLine(
    //         source->getPosition(),
    //         candidates[i].first->getPosition(),
    //         m_data.curvature.first,
    //         m_data.lineSegments.first);
    //
    //       line->setWidth(m_data.lineThickness.first);
    //
    //       if ( m_data.useParticleColors.first )
    //       {
    //         LineHelper::updateLineColors( line,
    //           source,
    //           candidates[i].first,
    //           m_data.invertColorTime.first );
    //       }
    //       else
    //       {
    //         LineHelper::updateCustomLineColors(
    //           line,
    //           candidates[i].first,
    //           source,
    //           m_data.lineColor.first,
    //           m_data.otherLineColor.first,
    //           m_data.invertColorTime.first );
    //       }
    //
    //       outArtifacts.push_back(line);
    //     }
    //   }
    // }

  private:
    static sf::Vector2f normalize(const sf::Vector2f& v)
    {
      const float len = std::sqrt(v.x * v.x + v.y * v.y);
      return len > 0.f ? v / len : sf::Vector2f{1.f, 0.f};
    }

    static float length(const sf::Vector2f& v)
    {
      return std::sqrt(v.x * v.x + v.y * v.y);
    }

    static float dot(const sf::Vector2f& a, const sf::Vector2f& b)
    {
      return a.x * b.x + a.y * b.y;
    }

  private:

    PipelineContext& m_ctx;
    KnnMeshData_t m_data;
  };

}