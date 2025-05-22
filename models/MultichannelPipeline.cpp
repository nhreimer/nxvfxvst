#include "models/MultichannelPipeline.hpp"

#include "channel/MidiChannelPipeline.hpp"
#include "data/PipelineContext.hpp"
#include "models/EventRecorder.hpp"
#include "models/encoder/EncoderFactory.hpp"
#include "shapes/TimedMessage.hpp"

#include "vst/analysis/FFTBuffer.hpp"

namespace nx
{
  MultichannelPipeline::MultichannelPipeline( PipelineContext& context )
    : m_ctx( context )
  {
    // set up the audio data channel pipeline, which is the first one
    m_channels[ AUDIO_CHANNEL_INDEX ] = std::make_unique< AudioChannelPipeline >( context, 0 );
    m_channelWorkers[ AUDIO_CHANNEL_INDEX ] = std::make_unique< ChannelWorker >(
      [ this ]
      {
        m_channels[ AUDIO_CHANNEL_INDEX ]->runTasks();
      } );

    // set up the midi channel pipelines
    for ( int32_t i = MIDI_CHANNEL_INDEX; i < m_channels.size(); ++i )
    {
      m_channels[ i ] = std::make_unique< MidiChannelPipeline >( context, i );
      m_channelWorkers[ i ] = std::make_unique< ChannelWorker >(
       [ this, i ]
       {
         // run all the tasks but ONLY on this thread
         m_channels[ i ]->runTasks();
       } );
    }
    m_messageClock.setMessage( "...welcome to nxvst..." );
  }

  [[nodiscard]]
  nlohmann::json MultichannelPipeline::saveState() const
  {
    nlohmann::json j = nlohmann::json::array();

    for ( int i = 0; i < m_channels.size(); ++i )
    {
      j[ i ] = m_channels[ i ]->saveChannelPipeline();
      LOG_INFO( j[ i ].dump() );
    }

    return j;
  }

  void MultichannelPipeline::restoreState( const nlohmann::json &j ) const
  {
    for ( int i = 0; i < j.size(); ++i )
      m_channels[ i ]->loadChannelPipeline( j.at( i ) );
  }

  void MultichannelPipeline::processMidiEvent( const Midi_t &midi ) const
  {
    const auto midiChannel = midi.channel + MIDI_CHANNEL_INDEX;

    if ( midiChannel < m_channels.size() )
    {
      static_cast< MidiChannelPipeline * >( m_channels.at( midiChannel ).get() )->processMidiEvent( midi );
    }

    if ( m_encoder ) m_encoder->addMidiEvent( midi );
  }

  void MultichannelPipeline::processAudioData( FFTBuffer& buffer )
  {
    static_cast< AudioChannelPipeline * >( m_channels.at( AUDIO_CHANNEL_INDEX ).get() )->processAudioBuffer( buffer );
    m_audioDataAverage.addSample( static_cast< double >( buffer.age().count() ) );
  }

  void MultichannelPipeline::draw( sf::RenderWindow &window )
  {
    m_totalRenderAverage.startTimer();

    // add a render update request and then start all the channel pipelines
    for ( int i = 0; i < m_channels.size(); ++i )
    {
      // skip the channel if it's bypassed
      if ( m_channels[ i ]->isBypassed() ) continue;

      m_channels[ i ]->requestRenderUpdate();
      m_channelWorkers[ i ]->requestPipelineRun();

      m_drawingPrioritizer.emplace( ChannelDrawingData_t
      {
        .priority = m_channels[ i ]->getDrawPriority(),
        .channel = m_channels[ i ].get(),
        .channelWorker = m_channelWorkers[ i ].get()
      } );
    }

    // wait for the pipelines to finish and draw each item in order
    while ( !m_drawingPrioritizer.empty() )
    {
      const auto& top = m_drawingPrioritizer.top();
      top.channelWorker->waitUntilComplete();
      const auto * texture = top.channel->getOutputTexture();
      if ( texture != nullptr )
      {
        window.draw( sf::Sprite( texture->getTexture() ),
                     top.channel->getChannelBlendMode() );
      }
      // else
      // {
      //   LOG_ERROR( "render thread draw failed" );
      // }

      // must pop no matter what or an infinite loop will occur
      m_drawingPrioritizer.pop();
    }

    // now that we have a final image, send it to the video encoder
    // and make sure we start at the right time
    if ( m_encoder )
    {
      if ( m_encoder->isRecording() ) m_encoder->writeFrame( m_ctx.globalInfo.playhead, window );
      else
      {
        m_messageClock.setMessage( "Encoder failed. NOT recording." );
        m_encoder.reset( nullptr );
      }
    }

    // video encoding gets added whenever available,
    // but there's no separate one right now for video encoding
    // unless it becomes a problem
    m_totalRenderAverage.stopTimerAndAddSample();
  }

  void MultichannelPipeline::update( const sf::Time &deltaTime )
  {
    m_frameDiagnostics.update( deltaTime );

    for ( const auto& channel: m_channels )
      channel->update( deltaTime );
  }

  void MultichannelPipeline::shutdown() const
  {
    LOG_INFO( "Issuing shutdown requests..." );
    for ( int i = 0; i < m_channels.size(); ++i )
    {
      m_channels[ i ]->requestShutdown();
      m_channelWorkers[ i ]->requestPipelineRun();
      m_channelWorkers[ i ]->waitUntilComplete();
      LOG_INFO( "Channel {} shutdown completed", i + 1 );
    }

    LOG_INFO( "Shutdown requests completed" );
  }

  void MultichannelPipeline::drawMenu()
  {
    drawPipelineMenu();
    drawPipelineMetrics();
  }

  void MultichannelPipeline::drawPipelineMenu()
  {
    // semi-transparent background
    ImGui::SetNextWindowBgAlpha( m_mainWindowOpacity );

    ImGui::Begin(
      FULL_PRODUCT_NAME,
      nullptr,
      ImGuiWindowFlags_AlwaysAutoResize );

    int32_t offsetChannel = m_selectedChannel + 1;

    ImGui::SliderFloat( "Window Opacity##1", &m_mainWindowOpacity, 0.f, 1.f );
    ImGui::Text( "BPM: %0.2f", m_ctx.globalInfo.bpm );
    ImGui::Text( "Playhead: %0.5f", m_ctx.globalInfo.playhead );

    // we do this so that the channel is 1 and not 0. it's just a convenience thing
    // but please note that the channel is 0-based index!
    if ( ImGui::SliderInt( "##ChannelSelector",
      &offsetChannel,
      1,
      static_cast< int32_t >( m_channels.size() ),
      "Channel %d" ) )
    {
      m_selectedChannel = offsetChannel - 1;
    }

    ImGui::Separator();

    m_channels[ m_selectedChannel ]->drawMenu();

    ImGui::Separator();
    if ( ImGui::Button( "export" ) )
    {
      const auto json = saveState();
      ImGui::SetClipboardText( json.dump().c_str() );
      m_messageClock.setMessage( "data copied to clipboard." );
    }
    ImGui::SameLine();
    if ( ImGui::Button( "import" ) )
    {
      const auto json = std::string( ImGui::GetClipboardText() );
      if ( !json.empty() )
      {
        const auto importedData =
          nlohmann::json::parse( json.c_str(), nullptr, false );

        if ( !importedData.is_discarded() )
        {
          restoreState( importedData );
          m_messageClock.setMessage( "data copied from clipboard." );
        }
        else
          m_messageClock.setMessage( "failed to import: invalid json." );
      }
      else
        m_messageClock.setMessage( "failed to import: empty clipboard." );
    }

    if ( ImGui::TreeNode( "Video Encoder" ) )
    {

      if ( ImGui::RadioButton( "RawRGBA", m_encoderType == E_Encoder::E_RawRGBA ) )
      {
        m_encoderType = E_Encoder::E_RawRGBA;
      }

      ImGui::InputText( "Filename ",
                        m_encoderData.outputFilename.data(),
                        m_encoderData.outputFilename.size() );

      ImGui::InputFloat( "Start at (seconds): ", &m_encoderData.startAtInSeconds );

      if ( m_encoder )
      {
        if ( ImGui::Button( "Stop Recording" ) )
        {
          m_encoder.reset( nullptr );
          m_messageClock.setMessage( "RECORDING STOPPED!" );
        }
      }
      else if ( ImGui::Button( "Start Recording" ) )
      {
        m_encoderData.size = m_ctx.globalInfo.windowSize;
        m_encoder.reset( nullptr );
        m_encoder = EncoderFactory::create( m_encoderType, m_encoderData );
        if ( m_encoder && m_encoder->isRecording() )
        {
          m_messageClock.setMessage( "RECORDING STARTED!" );
        }
      }

      ImGui::TreePop();
      ImGui::Spacing();
    }

    if ( !m_messageClock.hasExpired() )
    {
      ImGui::Separator();
      ImGui::TextUnformatted( m_messageClock.getMessage().data() );
    }

    ImGui::Separator();
    ImGui::Text( "%s", BUILD_NUMBER );
    ImGui::End();
  }

  void MultichannelPipeline::drawPipelineMetrics()
  {
    // semi-transparent background
    ImGui::SetNextWindowBgAlpha( m_metricsWindowOpacity );

    ImGui::Begin("Frame Diagnostics", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav);
    {
      ImGui::SliderFloat( "Window Opacity##2", &m_metricsWindowOpacity, 0.075f, 1.f );
      ImGui::Text( "Window Size: %d, %d", m_ctx.globalInfo.windowSize.x, m_ctx.globalInfo.windowSize.y );

      ImGui::SeparatorText( "Render Times (Avg)" );

      for ( int32_t i = 0; i < m_channelWorkers.size(); ++i )
      {
        const auto metrics = m_channelWorkers[ i ]->getMetrics();
        if ( i == AUDIO_CHANNEL_INDEX )
          ImGui::Text( "(Audio) Channel %d: %0.2f ms", i, metrics );
        else
          ImGui::Text( "(Midi) Channel %d: %0.2f ms", i, metrics );
      }

      ImGui::Text( "Total Time: %0.2f ms", m_totalRenderAverage.getAverage() );
      ImGui::Text( "Cycle Time: %0.2f ms", m_totalRenderAverage.getCycleTimeInMs() );
      ImGui::Text( "Cycle Size: %d samples", RENDER_SAMPLES_COUNT );

      ImGui::SeparatorText( "Audio Buffer (Avg)" );

      ImGui::Text( "Buffer age: %0.2f ms", m_audioDataAverage.getAverage() );

      m_frameDiagnostics.drawMenu();
    }

    ImGui::End();
  }
} // namespace nx
