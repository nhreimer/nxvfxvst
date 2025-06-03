#pragma once

#include "utils/RingBufferAverager.hpp"

namespace nx
{
  class RingParticleVisualizer final : public IAudioVisualizer
  {

    struct RingParticleData_t
    {
      bool isActive = true;
      float gain = 10.f;
      float baseRadius = 100.f;
      float radiusMod = 100.f;
      float particleSize = 8.f;
      sf::Color colorStart = { 255, 0, 255 };
      sf::Color colorEnd = sf::Color::Cyan;

      // for when to not display data
      float threshold = 0.1f;
    };

  public:
    explicit RingParticleVisualizer( PipelineContext& ctx )
      : m_ctx( ctx )
    {}

    [[nodiscard]]
    nlohmann::json serialize() const override { return {}; }

    void deserialize(const nlohmann::json &j) override {}
    [[nodiscard]] E_AudioVisualizerType getType() const override
    {
      return E_AudioVisualizerType::E_RingParticleVisualizer;
    }

    void receiveUpdatedAudioBuffer( const IFFTResult& fftResult ) override
    {
      m_timedBuffer.startTimer();

      const auto& fft = fftResult.getSmoothedBuffer();

      constexpr float angleStep = NX_TAU / static_cast<float>(FFT_BINS);

      for ( auto i = 0; i < FFT_BINS; ++i )
      {
        if ( fft[ i ] < m_data.threshold )
        {
          if ( m_particles[ i ].getRadius() > 0.f )
            m_particles[ i ].setRadius( 0.f );

          continue;
        }

        const float mag = fft[i] * m_data.gain;
        updateMaxEnergy(mag);

        const auto normMag = std::clamp(mag / (m_recentMax + 1e-5f), 0.f, 1.f);
        const auto eased = squash(normMag);

        const float angle = i * angleStep;
        const float radius = m_data.baseRadius + eased * m_data.radiusMod;

        const sf::Vector2f pos =
        {
          m_ctx.globalInfo.windowHalfSize.x + std::cos( angle ) * radius,
          m_ctx.globalInfo.windowHalfSize.y + std::sin( angle ) * radius
        };

        auto& particle = m_particles[i];
        particle.setPosition( pos );
        particle.setRadius( m_data.particleSize );
        particle.setFillColor( ColorHelper::lerpColor(m_data.colorStart, m_data.colorEnd, mag) );
        particle.setOrigin( { m_data.particleSize, m_data.particleSize } );
      }

      m_timedBuffer.stopTimerAndAddSample();
    }

    sf::RenderTexture* draw( sf::RenderTexture* inTexture ) override
    {
      m_texture.ensureSize( m_ctx.globalInfo.windowSize );

      m_texture.clear( sf::Color::Transparent );

      for ( auto& p : m_particles )
        m_texture.draw( p, m_blendMode );

      m_texture.display();
      return m_texture.get();
    }

    void update( const sf::Time& ) override {}
    void drawMenu() override
    {
      if ( ImGui::TreeNode( "Ring Particle Options" ) )
      {
        ImGui::Checkbox( "Active", &m_data.isActive);
        ImGui::SliderFloat( "Threshold", &m_data.threshold, 0.f, 1.f );
        ImGui::SliderFloat( "Gain", &m_data.gain, 0.f, 20.f );
        ImGui::SliderFloat( "BaseRadius", &m_data.baseRadius, 0.f, static_cast< float >(m_ctx.globalInfo.windowSize.x) );
        ImGui::SliderFloat( "RadiusMod", &m_data.radiusMod, 0.f, 400.f );
        ImGui::SliderFloat( "ParticleSize", &m_data.particleSize, 0.f, 100.f );
        ColorHelper::drawImGuiColorEdit4( "Color Start", m_data.colorStart );
        ColorHelper::drawImGuiColorEdit4( "Color End", m_data.colorEnd );

        ImGui::Separator();
        ImGui::Text( "Processing time: %0.2f", m_timedBuffer.getAverage() );

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

    void destroyTextures() override { m_texture.destroy(); }
    sf::BlendMode& getBlendMode() override { return m_blendMode; }

  private:

    static float squash(const float x)
    {
      // Easing: easeOutExpo
      return 1.0f - std::pow(2.0f, -10.0f * x);
    }

    void updateMaxEnergy(const float mag)
    {
      // Smooth decay and track new spikes
      m_recentMax = std::lerp(m_recentMax, mag, 0.05f);
      if (mag > m_recentMax)
        m_recentMax = mag;
    }

  private:

    PipelineContext& m_ctx;
    LazyTexture m_texture;

    RingParticleData_t m_data;

    float m_recentMax { 0.1f };

    std::array< sf::CircleShape, FFT_BINS > m_particles;
    sf::BlendMode m_blendMode { sf::BlendAdd };

    RingBufferAverager m_timedBuffer;

  };

}