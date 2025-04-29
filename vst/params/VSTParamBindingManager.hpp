#pragma once

#include <functional>
#include <string>
#include <vector>

#include "log/Logger.hpp"
#include "models/IShader.hpp"
#include "public.sdk/source/vst/vstparameters.h"

namespace nx
{
  // Forward declaration
  using ShaderControlSetter = std::function<float(float)>; // normalized 0–1

  // Represents a single binding between a VST param and a Shader Control
  struct VSTParamBinding
  {
    IShader * owner { nullptr };
    int32_t vstParamID { -1 };        // 0..127
    std::string shaderControlName;    // e.g., "Brightness", "Sigma", etc.
    ShaderControlSetter setter;       // Function to call when param changes
    float lastValue { 0.f };          // last known value
    float minValue { 0.f };
    float maxValue { 0.f };
  };

  ///
  /// Whenever an IShader is instantiated, it attempts to bind automatically
  /// to any available parameter IDs that are left.
  /// Whenever an IShader is destroyed, it must unbind to keep the parameters
  /// available for others.
  class VSTParamBindingManager
  {
  public:

    VSTParamBindingManager() = default;

    int32_t registerBindableControl( IShader * owner,
                                  const std::string& controlName,
                                  const float minValue,
                                  const float maxValue,
                                  ShaderControlSetter&& setter)
    {
      const auto nextId = getAvailableVSTParamID();
      if ( nextId == -1 )
      {
        LOG_WARN( "All VST Param IDs taken. Failed to bind control." );
        return -1;
      }

      auto& bindable = m_bindings[ nextId ];
      bindable.owner = owner;
      bindable.setter = std::move( setter );
      bindable.vstParamID = nextId;
      bindable.shaderControlName = controlName;
      bindable.lastValue = 0.f;
      bindable.minValue = minValue;
      bindable.maxValue = maxValue;

      LOG_INFO( "Registered {} as VST Param ID: {}", controlName, nextId );
      return nextId;
    }

    void unregisterAllControlsOwnedBy( const IShader * owner )
    {
      for ( auto& binding : m_bindings )
      {
        if ( binding.owner == owner )
        {
          LOG_INFO( "Unregistering param ID: {} from {}", binding.vstParamID, binding.shaderControlName );
          resetBinding( binding );
        }
      }
    }

    void unregisterIndividualControl( const int32_t vstParamID )
    {
      resetBinding( m_bindings[ vstParamID ] );
    }

    void assignParamIDToControl( IShader * owner,
                                 const std::string& controlName,
                                 const int32_t vstParamID)
    {
      for (auto& control : m_bindings)
      {
        if (control.shaderControlName == controlName)
        {
          control.owner = owner;
          control.shaderControlName = controlName;
          control.vstParamID = vstParamID;
          return;
        }
      }
    }

    void clearAllBindings()
    {
      for ( auto& binding : m_bindings ) resetBinding( binding );
    }

    void setParamNormalized(const int32_t vstParamID,
                            const float normalizedValue)
    {
      auto& binding = m_bindings[ vstParamID ];
      if ( binding.setter )
      {
        binding.lastValue = binding.setter( normalizedValue );
      }
    }

    static float getParamDenormalized( const VSTParamBinding& binding, const float normalizedValue )
    {
      return ( binding.maxValue - binding.minValue ) / normalizedValue;
    }

    [[nodiscard]]
    const auto& getBindings() const { return m_bindings; }

    [[nodiscard]]
    const auto& getBindingById( const int32_t paramID ) const { return m_bindings[ paramID ]; }

    template < typename T >
    void setValue( const int32_t vstParamID,
                   const T value )
    {
      if ( vstParamID > -1 )
      {
        if constexpr (std::is_same_v<T, float>)
        {
          m_bindings[ vstParamID ].lastValue = value;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
          m_bindings[ vstParamID ].lastValue = ( value > 0.f ) ? 1.f : 0.f;
        }
      }
    }

  private:

    int32_t getAvailableVSTParamID()
    {
      constexpr int maxParams = 128;
      for ( int32_t i = m_nextAvailableVSTParamID,
            y = 0; y < maxParams; i = i + 1 % maxParams, ++y )
      {
        if ( m_bindings[ i ].vstParamID == -1 )
        {
          m_nextAvailableVSTParamID = i;
          return i;
        }
      }

      return -1;
    }

    static void resetBinding( VSTParamBinding& binding )
    {
      binding.owner = nullptr;
      binding.shaderControlName = m_emptyString;
      binding.vstParamID = -1;
      binding.lastValue = 0.f;
      binding.setter = nullptr;
    }

  private:

    int32_t m_nextAvailableVSTParamID { 0 };
    std::array< VSTParamBinding, 128 > m_bindings;
    inline static std::string m_emptyString = "";
    // std::vector<VSTParamBinding> m_bindings; // key = VST Param ID
  };
}