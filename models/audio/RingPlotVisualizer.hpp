#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <cmath>
#include <imgui.h>

#include "models/IAudioVisualizer.hpp"
#include "utils/RingBufferAverager.hpp"

namespace nx
{
  class RingPlotVisualizer final : public IAudioVisualizer
  {

    struct RingPlotData_t
    {
      bool isActive = true;
      int32_t ringCount = 5;
      int32_t pointSmoothness = 64;
      float ringSpacing = 40.f;
      float baseRadius = 20.f;
      float gain = 1.f;
      float radiusMod = 100.f;
      float colorGain = 1.f;
      bool useColorGain = false;
      sf::Color colorStart = sf::Color::Cyan;
      sf::Color colorEnd = sf::Color::Red;
    };

  public:

    explicit RingPlotVisualizer( PipelineContext& ctx )
      : m_ctx(ctx)
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_AudioVisualizerType getType() const override { return E_AudioVisualizerType::E_RingPlotVisualizer; }

    sf::RenderTexture* draw(sf::RenderTexture* inTexture) override
    {
      m_texture.ensureSize( m_ctx.globalInfo.windowSize );
      m_texture.clear(sf::Color::Transparent);

      for (const auto& ring : m_drawables)
        m_texture.draw(ring, m_blendMode);

      m_texture.display();
      return m_texture.get();
    }

    void destroyTextures() override
    {
      m_texture.destroy();
    }

    void update(const sf::Time& deltaTime) override {}

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Ring Plot Options" ) )
      {
        ImGui::Checkbox("Enabled", &m_data.isActive);
        ImGui::SliderInt("Ring Count", &m_data.ringCount, 1, 16);
        ImGui::SliderInt("Points per Ring", &m_data.pointSmoothness, 16, 256);
        ImGui::SliderFloat("Ring Spacing", &m_data.ringSpacing, 5.f, 100.f);
        ImGui::SliderFloat("Gain", &m_data.gain, 0.01f, 10.f);
        ImGui::SliderFloat("Radius Mod", &m_data.radiusMod, 0.f, 300.f);
        ImGui::Checkbox( "Use Color Gain", &m_data.useColorGain );
        ImGui::SliderFloat( "Color Gain", &m_data.colorGain, 0.f, 10.f );
        ColorHelper::drawImGuiColorEdit4( "Color Start", m_data.colorStart );
        ColorHelper::drawImGuiColorEdit4( "Color End", m_data.colorEnd );

        ImGui::SeparatorText( "Audio Processing Time" );
        ImGui::Text( "Time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void receiveUpdatedAudioBuffer(const IFFTResult& fftResult) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();
      // const auto size = m_ctx.globalInfo.windowSize;
      //const sf::Vector2f center = { ( float )size.x / 2.f, ( float )size.y / 2.f };

      m_drawables.clear();
      const int32_t binsPerRing = FFT_BINS / m_data.ringCount;

      for (int32_t ringIndex = 0; ringIndex < m_data.ringCount; ++ringIndex)
      {
        sf::VertexArray ring(sf::PrimitiveType::LineStrip);
        const float baseRadius = m_data.baseRadius + static_cast< float >(ringIndex) * m_data.ringSpacing;

        const auto start = ringIndex * binsPerRing;
        const auto end = std::min(start + binsPerRing, FFT_BINS);
        const auto subdivisions = m_data.pointSmoothness;

        float firstMag = 0.f;

        for (size_t i = 0; i <= subdivisions; ++i)
        {
          const float t = static_cast<float>(i) / static_cast<float>(subdivisions);
          const float angle = t * NX_TAU;

          const float binT = t * static_cast<float>(end - start);
          int32_t binIndex = (i == subdivisions) ? start : start + static_cast<int32_t>(binT);
          binIndex = std::clamp(binIndex, 0, static_cast<int32_t>(FFT_BINS) - 1);

          float mag = fft[binIndex] * m_data.gain;
          if (!std::isfinite(mag) || mag > 1000.f)
            mag = 0.f;

          if (i == 0) firstMag = mag;
          if (i == subdivisions) mag = firstMag;

          float radius = baseRadius + mag * m_data.radiusMod;
          if (!std::isfinite(radius) || radius > 10000.f)
            radius = baseRadius;

          const sf::Vector2f point =
          {
            m_ctx.globalInfo.windowHalfSize.x + std::cos(angle) * radius,
            m_ctx.globalInfo.windowHalfSize.y + std::sin(angle) * radius
          };

          sf::Color color;
          if ( m_data.useColorGain )
          {
            const float normalizedMag = std::clamp(mag / m_data.colorGain, 0.f, 1.f);
            color = ColorHelper::lerpColor(m_data.colorStart, m_data.colorEnd, normalizedMag);
          }
          else
          {
            color = ColorHelper::lerpColor(
              m_data.colorStart, m_data.colorEnd,
              static_cast<float>(ringIndex) / static_cast<float>(m_data.ringCount)
            );
          }
          ring.append(sf::Vertex(point, color));
        }

        m_drawables.push_back(ring);
      }

      m_timedBuffer.stopTimerAndAddSample();
    }


    sf::BlendMode& getBlendMode() override { return m_blendMode; }

  private:

    PipelineContext& m_ctx;
    RingPlotData_t m_data;
    LazyTexture m_texture;
    std::vector<sf::VertexArray> m_drawables;

    sf::BlendMode m_blendMode = sf::BlendAdd;

    RingBufferAverager m_timedBuffer;

  };

}