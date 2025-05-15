#pragma once

#include "models/data/PipelineContext.hpp"

#include "models/particle/behavior/ColorMorphBehavior.hpp"
#include "models/particle/behavior/FreeFallBehavior.hpp"
#include "models/particle/behavior/JitterBehavior.hpp"
#include "models/particle/behavior/MagneticBehavior.hpp"

#include "models/shader/BlurShader.hpp"

namespace nx
{

  class SerialGenerator final
  {
  public:

    SerialGenerator()
      : m_pipelineContext( { m_globalInfo, m_vstState } )
    {
      // initializeBehaviors();
      initializeShaders();
    }

    void drawMenu() const
    {
      ImGui::Begin("Serial Generator",
                    nullptr,
                    ImGuiWindowFlags_AlwaysAutoResize );
      {
        //drawMenus< IParticleBehavior >( "Behavior Serializers", m_behaviorSerializers );
        drawMenus< IShader >( "Shader Serializers", m_shaderSerializers );
      }

      ImGui::End();
    }

  private:

    template < typename T >
    void addBehavior()
    {
      m_behaviorSerializers.emplace_back( std::make_unique< T >( m_pipelineContext ) );
    }

    void initializeBehaviors()
    {
      addBehavior<ColorMorphBehavior>();
      addBehavior<FreeFallBehavior>();
      addBehavior<JitterBehavior>();
      addBehavior<MagneticAttractorBehavior>();
    }

    template < typename T >
    void addShader()
    {
      m_shaderSerializers.emplace_back( std::make_unique< T >( m_pipelineContext ) );
    }

    void initializeShaders()
    {
      addShader<BlurShader>();
    }

    template < typename T, typename TEnum  >
    void drawMenus(
      const std::string& serializerTypeName,
      const std::vector< std::unique_ptr< ISerializable< TEnum > > >& serializers ) const
    {
      if ( ImGui::TreeNode( serializerTypeName.c_str() ) )
      {
        for ( int i = 0; i < serializers.size(); ++i )
        {
          if ( i > 0 )
            ImGui::Separator();

          auto * ptr = serializers[ i ].get();

          ImGui::PushID( ptr );
          {
            if ( ImGui::SmallButton( "copy" ) )
              serializeToClipboard( ptr );

            ImGui::SameLine();
            if ( ImGui::SmallButton( "paste" ) )
              deserializeFromClipboard( ptr );

            auto * behavior = static_cast< T * >( ptr );
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
      LOG_INFO( json.dump() );
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

    ///////////////////////////////////////////////////////
    /// DATA SETUP

    std::vector< std::unique_ptr< ISerializable< E_BehaviorType > > > m_behaviorSerializers;
    std::vector< std::unique_ptr< ISerializable< E_ShaderType > > > m_shaderSerializers;

  };

}