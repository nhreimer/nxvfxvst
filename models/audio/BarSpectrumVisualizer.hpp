#pragma once

#include "models/IAudioVisualizer.hpp"
#include "models/data/PipelineContext.hpp"
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
      m_texture.clear( sf::Color::Transparent );

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
        ImGui::SliderFloat("Gain", &m_data.gain, 0.1f, 10.f);
        ImGui::SliderFloat("Falloff Speed", &m_data.falloffSpeed, 0.8f, 0.9999f);
        ColorHelper::drawImGuiColorEdit4("Bar Color", m_data.barColor);
        MenuHelper::drawBlendOptions( m_blendMode );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void receiveUpdatedAudioBuffer( const AudioDataBuffer& fft ) override
    {
      const auto size = m_context.globalInfo.windowSize;

      const float barWidth = (size.x - (m_data.barCount - 1) * m_data.barSpacing) / m_data.barCount;

      for ( size_t i = 0; i < m_data.barCount; ++i )
      {
        auto& bar = m_bars[ i ];

        const auto binStart = i * (FFT_BINS / m_data.barCount);
        const auto binEnd = (i + 1) * (FFT_BINS / m_data.barCount);
        float energy = 0.f;

        for ( size_t b = binStart; b < binEnd; ++b )
          energy += fft[ b ];

        energy /= ( binEnd - binStart );

        float barHeight = energy * m_data.gain * size.y;
        m_barHeights[ i ] = std::max(m_barHeights[i], barHeight);

        //sf::RectangleShape bar;
        bar->setSize({barWidth, -m_barHeights[ i ]});
        bar->setPosition({ i * (barWidth + m_data.barSpacing),
                              static_cast< float >(size.y) });

        if ( m_data.barColor != bar->getFillColor() )
          bar->setFillColor( m_data.barColor );
      }
    }

    void update( const sf::Time &deltaTime ) override
    {
      // m_data.decayFactor = std::pow(m_data.falloffSpeed, deltaTime.asSeconds());
      // for ( size_t i = 0; i < m_data.barCount; ++i )
      // {
      //   m_barHeights[ i ] *= m_data.decayFactor;
      // }
    }

    sf::BlendMode &getBlendMode() override { return m_blendMode; }

    bool isEnabled() const override { return m_data.isEnabled; }
    void setEnabled( const bool isEnabled ) override { m_data.isEnabled = isEnabled; }

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