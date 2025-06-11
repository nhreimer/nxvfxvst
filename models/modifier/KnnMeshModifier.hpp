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
    {}

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

    void modify(const ParticleLayoutData_t&,
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
          const float dist = distance(posA, b->getPosition());
          if (dist <= m_data.maxDistance.first)
            distances.emplace_back(dist, j);
        }

        std::ranges::sort(distances);

        const size_t limit = std::min<size_t>(m_data.kNeighbors.first, distances.size());
        for (size_t k = 0; k < limit; ++k)
        {
          const auto& [dist, j] = distances[k];
          const auto* b = particles[j];

          // auto * line = new sf::VertexArray(sf::PrimitiveType::Lines, 2);
          // (*line)[0].position = posA;
          // (*line)[1].position = b->getPosition();
          //
          // outArtifacts.push_back(line);

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

  private:
    static float distance(const sf::Vector2f& a, const sf::Vector2f& b)
    {
      const auto dx = a.x - b.x;
      const auto dy = a.y - b.y;
      return std::sqrt(dx * dx + dy * dy);
    }

    PipelineContext& m_ctx;
    KnnMeshData_t m_data;
  };

}