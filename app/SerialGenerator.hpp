#pragma once

#include "models/data/PipelineContext.hpp"

#include "models/particle/behavior/ColorMorphBehavior.hpp"
#include "models/particle/behavior/FreeFallBehavior.hpp"
#include "models/particle/behavior/JitterBehavior.hpp"
#include "models/particle/behavior/MagneticBehavior.hpp"
#include "models/particle/behavior/RadialSpreaderBehavior.hpp"

namespace nx
{

  class SerialGenerator final
  {
  public:

    SerialGenerator()
      : m_pipelineContext( { m_globalInfo, m_vstState } )
    {

      initializeBehaviors();
    }

    void drawMenu() const
    {
      ImGui::Begin("Serial Generator",
                    nullptr,
                    ImGuiWindowFlags_AlwaysAutoResize );
      {
        drawBehaviorMenus();
      }

      ImGui::End();
    }

  private:

    template < typename T >
    void addBehavior( const int32_t position )
    {
      m_behaviorSerializers[ position ] =
        std::make_unique< T >( m_pipelineContext );
    }

    void initializeBehaviors()
    {
      addBehavior<ColorMorphBehavior>( 0 );
      addBehavior<FreeFallBehavior>( 1 );
      addBehavior<JitterBehavior>( 2 );
      addBehavior<MagneticAttractorBehavior>( 3 );
      addBehavior<RadialSpreaderBehavior>( 4 );
    }

    void drawBehaviorMenus() const
    {

      if ( ImGui::TreeNode( "Behavior Serialization" ) )
      {
        for ( int i = 0; i < m_behaviorSerializers.size(); ++i )
        {

          if ( i > 0 )
            ImGui::Separator();

          auto * ptr = m_behaviorSerializers[ i ].get();

          ImGui::PushID( ptr );
          {
            if ( ImGui::SmallButton( "copy" ) )
              serializeToClipboard( ptr );

            ImGui::SameLine();
            if ( ImGui::SmallButton( "paste" ) )
              deserializeFromClipboard( ptr );

            auto * behavior = static_cast< IParticleBehavior * >( ptr );
            behavior->drawMenu();
          }
          ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }
    }

  private:

    template < typename TEnum >
    static void serializeToClipboard( ISerializable< TEnum > * serializer )
    {
      const auto json = serializer->serialize();
      ImGui::SetClipboardText( json.dump().c_str() );
    }

    template < typename TEnum >
    static void deserializeFromClipboard( ISerializable< TEnum > * serializer )
    {
      const auto json = std::string( ImGui::GetClipboardText() );
      if ( !json.empty() )
      {
        const auto importedData =
          nlohmann::json::parse( json.c_str(), nullptr, false );

        if ( !importedData.is_discarded() )
        {
          serializer->deserialize( importedData );
          LOG_INFO( "data copied from clipboard." );
        }
        else
        {
          LOG_ERROR( "failed to import: invalid json." );
        }
      }
      else
      {
        LOG_ERROR( "failed to import: empty clipboard." );
      }
    }

  private:

    std::array< std::unique_ptr< ISerializable< E_BehaviorType > >, 5 > m_behaviorSerializers;

    ///////////////////////////////////////////////////////
    /// TEST SETUP

    inline static GlobalInfo_t m_globalInfo
    {
      .windowSize { 1280, 768 },
      .windowHalfSize { 1280.f / 2.f, 786.f / 2.f },
      .windowView {}
    };

    inline static VSTParamBindingManager m_paramManager
    {
      []( int32_t id, float f ) {},
      []( int32_t id ) {}
    };

    inline static VSTStateContext m_vstState
    {
      m_paramManager
    };

    PipelineContext m_pipelineContext;

  };

}