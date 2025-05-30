#pragma once

#include "models/IAudioVisualizer.hpp"
#include "models/data/PipelineContext.hpp"
#include "models/audio/IFFTResult.hpp"
// #include "models/shader/BlenderShader.hpp"
#include "utils/LazyTexture.hpp"

namespace nx
{
  class BarSpectrumVisualizer final : public IAudioVisualizer
  {
    struct BarSpectrumData_t
    {
      bool isEnabled { true };
      int32_t barCount = 64;
      float barSpacing = 4.f;
      float gain = 4.f;
      float falloffSpeed = 0.92f;
      float decayFactor = 1.0f;
      sf::Color barColor = sf::Color::Cyan;
    };

  public:
    explicit BarSpectrumVisualizer( PipelineContext& context )
      : m_context( context )
    {
      for ( int32_t i = 0; i < m_bars.size(); ++i )
        m_bars[ i ] = std::make_unique< sf::RectangleShape >();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return {};
    }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_AudioVisualizerType getType() const override
    {
      return E_AudioVisualizerType::E_BarSpectrumVisualizer;
    }

    sf::RenderTexture * draw( sf::RenderTexture * inTexture ) override
    {
      m_texture.ensureSize( m_context.globalInfo.windowSize );
      m_texture.clear( sf::Color::Transparent );

      // draw the previous content, if there is any available
      if ( inTexture )
      {
        m_texture.draw( sf::Sprite( inTexture->getTexture() ), m_blendMode );
      }

      // draw our content
      for ( size_t i = 0; i < m_data.barCount; ++i )
        m_texture.draw( *m_bars[ i ], m_blendMode );

      m_texture.display();
      return m_texture.get();
    }

    void destroyTextures() override
    {
      m_texture.destroy();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Bar Spectrum" ) )
      {
        ImGui::Text("Vertical Bars");
        ImGui::SliderInt("Bar Count", &m_data.barCount, 8, 256);
        ImGui::SliderFloat("Spacing", &m_data.barSpacing, 0.f, 10.f);
        ImGui::SliderFloat("Gain", &m_data.gain, 0.1f, 20.f);
        ImGui::SliderFloat("Falloff Speed", &m_data.falloffSpeed, 0.8f, 0.9999f);
        ColorHelper::drawImGuiColorEdit4("Bar Color", m_data.barColor);
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void receiveUpdatedAudioBuffer(const IFFTResult& fftResult) override
    {
      const auto& size = m_context.globalInfo.windowSize;

      const float barWidth = (size.x - (m_data.barCount - 1) * m_data.barSpacing) / m_data.barCount;
      const auto& fft = fftResult.getSmoothedBuffer();

      const size_t usableBars = std::min(m_data.barCount, ( int32_t )FFT_BINS);

      for (size_t i = 0; i < usableBars; ++i)
      {
        auto& bar = m_bars[i];

        const size_t binStart = i * (FFT_BINS / usableBars);
        const size_t binEnd   = (i + 1) * (FFT_BINS / usableBars);

        if (binEnd <= binStart)
          continue;

        float energy = 0.f;
        for (size_t b = binStart; b < binEnd; ++b)
          energy += fft[b];

        energy /= static_cast<float>(binEnd - binStart);

        // Optional debug clamp to help with noise floors or excessive gain
        energy = std::clamp(energy, 0.f, 1.f);

        // Compute new height with gain multiplier
        const float barHeight = energy * m_data.gain * size.y;

        // You can choose to decay or snap (this version snaps directly)
        m_barHeights[i] = barHeight;

        // Update bar geometry
        bar->setSize({ barWidth, -m_barHeights[i] }); // assumes bottom origin

        bar->setPosition({
            i * (barWidth + m_data.barSpacing),
            static_cast<float>(size.y)
        });

        if (m_data.barColor != bar->getFillColor())
          bar->setFillColor(m_data.barColor);

        // Optional: debug log
        // LOG_INFO("Bar[{}] bins[{}..{}] energy {:.3f} height {:.2f}", i, binStart, binEnd, energy, m_barHeights[i]);
      }

      // Optional: zero out unused bars if barCount > FFT_BINS
      for (size_t i = usableBars; i < m_data.barCount; ++i)
      {
        m_barHeights[i] = 0.f;
        m_bars[i]->setSize({ barWidth, 0.f });
      }
    }

    void update( const sf::Time &deltaTime ) override
    {
      m_data.decayFactor = std::pow( m_data.falloffSpeed, deltaTime.asSeconds() );
      for ( size_t i = 0; i < m_data.barCount; ++i )
      {
        m_barHeights[ i ] *= m_data.decayFactor;
      }
    }

    sf::BlendMode &getBlendMode() override { return m_blendMode; }

  private:

    PipelineContext& m_context;
    BarSpectrumData_t m_data;

    LazyTexture m_texture;
    // BlenderShader m_blenderShader;

    sf::BlendMode m_blendMode { sf::BlendAdd };
    std::array< float, 256 > m_barHeights { 0.f };
    std::array< std::unique_ptr< sf::RectangleShape >, 256 > m_bars;
  };
}