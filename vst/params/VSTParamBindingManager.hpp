/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <complex>
#include <functional>
#include <string>
#include <vector>

#include "helpers/Definitions.hpp"
#include "log/Logger.hpp"
#include "public.sdk/source/vst/vstparameters.h"

namespace nx
{
  // normalized 0–1
  using ShaderControlSetter = std::function<float(float)>;
  using OnControlRegistrationCallback = std::function<void(int32_t, float)>;
  using OnControlUnRegistrationCallback = std::function<void(int32_t)>;

  // Represents a single binding between a VST param and a Shader Control
  struct VSTParamBinding
  {
    void * owner { nullptr };
    int32_t vstParamID { -1 };        // e.g., 0..127
    std::string shaderControlName;    // e.g., "Brightness", "Sigma", etc.
    ShaderControlSetter setter;       // Function to call when param changes
    float lastValue { 0.f };          // last known value
    float minValue { 0.f };
    float maxValue { 0.f };
  };

  ///
  /// Whenever a controller is instantiated, it attempts to bind automatically
  /// to any available parameter IDs that are left.
  /// Whenever a controller is destroyed, it must unbind to keep the parameters
  /// available for others.
  class VSTParamBindingManager
  {
  public:

    VSTParamBindingManager( OnControlRegistrationCallback&& onRegistrationCallback,
                            OnControlUnRegistrationCallback&& onUnregistrationCallback )
      : m_onRegistrationCallback( std::move( onRegistrationCallback ) ),
        m_onUnRegistrationCallback( std::move( onUnregistrationCallback ) )
    {}

    int32_t registerBindableControl( void * owner,
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

    void unregisterAllControlsOwnedBy( const void * owner )
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

    void assignParamIDToControl( void * owner,
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
        // this is the denormalized value we get back
        binding.lastValue = binding.setter( normalizedValue );
      }
    }

    int32_t findParamID( const void * owner, const std::string& controlName ) const
    {
      for ( auto& binding : m_bindings )
      {
        if ( binding.owner == owner && binding.shaderControlName == controlName )
          return binding.vstParamID;
      }

      return -1;
    }

    static float convertToDenormalized( const VSTParamBinding& binding, const float normalizedValue )
    {
      // we convert the normalized for display purposes
      // ( 50 - 0 ) * 0.14 = 7
      return ( binding.maxValue - binding.minValue ) * normalizedValue;
    }

    static float convertToNormalized( const VSTParamBinding& binding, const float unnormalizedValue )
    {
      // we use the range parameter, so it requires 0 - 1
      // 7 / ( 50 - 0 ) = 0.14
      return unnormalizedValue / ( binding.maxValue - binding.minValue );
    }

    [[nodiscard]]
    const auto& getBindings() const { return m_bindings; }

    [[nodiscard]]
    const auto& getBindingById( const int32_t paramID ) const
    {
      return m_bindings[ paramID ];
    }

    template < typename T >
    void setValue( const int32_t vstParamID,
                   const T value )
    {
      if ( vstParamID > -1 )
      {
        if constexpr (std::is_same_v<T, float>)
        {
          m_bindings[ vstParamID ].lastValue = convertToNormalized( m_bindings[ vstParamID ], value );

          // force update in the controller
          m_onRegistrationCallback(
            vstParamID,
            m_bindings[ vstParamID ].lastValue );
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
          m_bindings[ vstParamID ].lastValue = ( value > 0.f ) ? 1.f : 0.f;
          m_onRegistrationCallback( vstParamID, m_bindings[ vstParamID ].lastValue );
        }
      }
    }

  private:

    int32_t getAvailableVSTParamID()
    {
      for ( int32_t i = m_nextAvailableVSTParamID,
            y = 0; y < PARAMETERS_ENABLED; i = i + 1 % PARAMETERS_ENABLED, ++y )
      {
        if ( m_bindings[ i ].vstParamID == -1 )
        {
          m_nextAvailableVSTParamID = i;
          return i;
        }
      }

      return -1;
    }

    void resetBinding( VSTParamBinding& binding ) const
    {
      m_onUnRegistrationCallback( binding.vstParamID );
      binding.owner = nullptr;
      binding.shaderControlName = m_emptyString;
      binding.vstParamID = -1;
      binding.lastValue = 0.f;
      binding.setter = nullptr;
    }

  private:

    OnControlRegistrationCallback m_onRegistrationCallback;
    OnControlUnRegistrationCallback m_onUnRegistrationCallback;
    int32_t m_nextAvailableVSTParamID { 0 };
    std::array< VSTParamBinding, PARAMETERS_ENABLED > m_bindings;
    inline static std::string m_emptyString;
    // std::vector<VSTParamBinding> m_bindings; // key = VST Param ID
  };
}