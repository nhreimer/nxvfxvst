#pragma once

#include "models/IAudioVisualizer.hpp"

namespace nx
{

  class RingBarVisualization final : public IAudioVisualizer
  {
    struct RingBarData_t
    {
      bool isEnabled { true };
      int32_t barCount = 64;
      float radius = 100.f;
      float gain = 12.f;
      float barThickness = 4.f;
      sf::Color barColor = sf::Color::Green;
    };
  public:

    explicit RingBarVisualization( PipelineContext& ctx )
      : m_ctx( ctx )
    {
      verifyCapacity();
    }

    [[nodiscard]]
    nlohmann::json serialize() const override
    {
      return {};
    }

    void deserialize(const nlohmann::json &j) override {}

    [[nodiscard]]
    E_AudioVisualizerType getType() const override { return E_AudioVisualizerType::E_RingBarVisualizer; }

    sf::RenderTexture * draw(sf::RenderTexture *inTexture) override
    {
      m_texture.ensureSize( m_ctx.globalInfo.windowSize );
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
      if ( ImGui::TreeNode( "Ring Bar Options" ) )
      {
        if ( ImGui::SliderInt( "Bar Count##1", &m_data.barCount, 1, 128 ) )
        {
          verifyCapacity();
        }

        ImGui::SliderFloat( "Radius##2", &m_data.radius, 0.0f, 200.0f );
        ImGui::SliderFloat( "Gain##3", &m_data.gain, 0.0f, 20.0f );
        ImGui::SliderFloat( "Bar Thickness##4", &m_data.barThickness, 0.0f, 20.0f );

        ColorHelper::drawImGuiColorEdit4( "Bar Color", m_data.barColor );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void receiveUpdatedAudioBuffer(const IFFTResult &fftResult ) override
    {
      const auto& size = m_ctx.globalInfo.windowSize;
      const auto& fft = fftResult.getSmoothedBuffer();

      const sf::Vector2f center =
      {
        size.x * 0.5f,
        size.y * 0.5f
      };

      const size_t usableBars = std::min(m_data.barCount, static_cast< int32_t >(FFT_BINS));
      const float angleStepDeg = 360.f / static_cast<float>(usableBars);

      for (size_t i = 0; i < usableBars; ++i)
      {
        const size_t binStart = i * (FFT_BINS / usableBars);
        const size_t binEnd = (i + 1) * (FFT_BINS / usableBars);

        if (binEnd <= binStart)
          continue;

        float energy = 0.f;
        for (size_t b = binStart; b < binEnd; ++b)
          energy += fft[b];

        energy /= static_cast<float>(binEnd - binStart);
        energy = std::clamp(energy, 0.f, 1.f);

        const float barLength = energy * m_data.gain * m_ctx.globalInfo.windowSize.y;

        auto& bar = m_bars[i];
        const float angleDeg = i * angleStepDeg;
        const float angleRad = angleDeg * NX_D2R;

        // Position bar at circle edge
        const sf::Vector2f start =
        {
          center.x + std::cos(angleRad) * m_data.radius,
          center.y + std::sin(angleRad) * m_data.radius
        };

        // Update geometry
        bar->setSize({ m_data.barThickness, -barLength }); // negative for outward
        bar->setOrigin({ m_data.barThickness * 0.5f, 0.f });    // pivot from base
        bar->setPosition( start );
        bar->setRotation( sf::degrees( angleDeg ) );
        bar->setFillColor( m_data.barColor );
      }

      // Optional: clear unused bars if barCount > FFT_BINS
      for ( size_t i = usableBars; i < m_bars.size(); ++i )
      {
        m_bars[i]->setSize({ 0.f, 0.f });
      }
    }

    void update(const sf::Time &deltaTime) override
    {}

    sf::BlendMode & getBlendMode() override { return m_blendMode; };

    bool isEnabled() const override { return m_data.isEnabled; }
    void setEnabled(bool value) override { m_data.isEnabled = value; }

  private:

    void verifyCapacity()
    {
      if ( m_bars.size() < m_data.barCount )
      {
        for ( auto i = m_bars.size(); i < m_data.barCount; ++i )
        {
          const auto& shape = m_bars.emplace_back( std::make_unique<sf::RectangleShape>() );
          shape->setFillColor(m_data.barColor);
          shape->setOrigin({ m_data.barThickness * 0.5f, 0.f });
        }
      }
    }

  private:

    PipelineContext& m_ctx;
    LazyTexture m_texture;
    RingBarData_t m_data;
    sf::BlendMode m_blendMode { sf::BlendAdd };
    std::vector< std::unique_ptr< sf::RectangleShape > > m_bars;
  };

}