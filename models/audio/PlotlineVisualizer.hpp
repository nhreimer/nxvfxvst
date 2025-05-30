#pragma once

#include "resources/embedded_font.hpp"

namespace nx
{
  class PlotlineVisualizer final : public IAudioVisualizer
  {

    struct PlotLineVisualizerData_t
    {
      bool enabled = true;
      bool showGrid = true;
      sf::Color lineColor = sf::Color::Cyan;
      bool showSmoothed = true;
      float gain = 0.5f;
    };

  public:
    explicit PlotlineVisualizer( const PipelineContext& ctx )
      : m_ctx( ctx )
    {
      if ( m_debugFont.openFromMemory( Raleway_Regular_ttf, Raleway_Regular_ttf_len ) )
      {
        LOG_INFO( "successfully loaded font" );
      }
      else
      {
        LOG_ERROR( "failed to load font from file" );
      }
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
      return E_AudioVisualizerType::E_PlotLineVisualizer;
    }

    sf::RenderTexture * draw(sf::RenderTexture * inTexture) override
    {
      m_texture.ensureSize( m_ctx.globalInfo.windowSize );
      m_texture.clear( sf::Color::Transparent );

      // draw the previous content, if there is any available
      if ( inTexture )
      {
        m_texture.draw( sf::Sprite( inTexture->getTexture() ), m_blendMode );
      }

      m_texture.clear(sf::Color::Transparent);

      if (!m_fftBuffer) return m_texture.get();

      const auto& fft = m_data.showSmoothed
        ? m_fftBuffer->getSmoothedBuffer()
        : m_fftBuffer->getRealTimeBuffer();

      const sf::Vector2f size =
      {
        static_cast<float>(m_texture.getSize().x),
        static_cast<float>(m_texture.getSize().y)
      };

      const float binWidth = size.x / static_cast<float>(fft.size());

      sf::VertexArray lines(sf::PrimitiveType::LineStrip, fft.size());

      for (size_t i = 0; i < fft.size(); ++i)
      {
        const float x = i * binWidth;
        float y = size.y - (fft[i] * m_data.gain * size.y);
        y = std::clamp(y, 0.f, size.y);

        lines[i].position = sf::Vector2f(x, y);
        lines[i].color = m_data.lineColor;
      }

      drawBinGridOverlay();
      m_texture.draw(lines, m_blendMode);
      m_texture.display();
      return m_texture.get();
    }

    void destroyTextures() override
    {
      m_texture.destroy();
    }

    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Plotline Options" ) )
      {
        ImGui::Checkbox("Enabled", &m_data.enabled);
        ImGui::Checkbox("Show Grid", &m_data.showGrid);
        ImGui::Checkbox("Show Smoothed Buffer", &m_data.showSmoothed);
        ImGui::SliderFloat("Gain", &m_data.gain, 0.1f, 10.f);
        ColorHelper::drawImGuiColorEdit4( "Line Color", m_data.lineColor );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void receiveUpdatedAudioBuffer(const IFFTResult& fftResult) override
    {
      m_fftBuffer = &fftResult;
    }

    void update(const sf::Time&) override {}

    sf::BlendMode& getBlendMode() override { return m_blendMode; }

  private:

    void drawBinGridOverlay()
    {
      const float binWidthHz = m_ctx.globalInfo.sampleRate / static_cast<float>(FFT_SIZE);
      const float maxFreq = m_ctx.globalInfo.sampleRate / 2.f;

      const auto size = m_texture.getSize();

      for (int32_t i = 0; i < FFT_BINS; ++i)
      {
        const float freq = i * binWidthHz;
        float x = (freq / maxFreq) * size.x;


        CurvedLine line( { x, 0.f}, { x, static_cast< float >(size.y) }, 0.f );
        line.setGradient( sf::Color::Black, { 64, 64, 64 } );
        line.setWidth( 1.f );

        m_texture.draw( line, m_blendMode );

        // Label (every N bins to reduce clutter)
        if (i % 8 == 0)
        {
          std::ostringstream ss;
          ss << i << "\n" << static_cast<int>(freq) << "Hz";

          sf::Text label(m_debugFont, ss.str(), 16);
          label.setPosition({x + 2.f, 2.f});
          label.setFillColor(sf::Color::White);

          m_texture.draw(label, m_blendMode);
        }
      }
    }

  private:
    const PipelineContext& m_ctx;
    PlotLineVisualizerData_t m_data;
    LazyTexture m_texture;
    sf::BlendMode m_blendMode = sf::BlendAdd;

    const IFFTResult * m_fftBuffer = nullptr;
    sf::Font m_debugFont;
  };

}