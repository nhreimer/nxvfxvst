#pragma once

#include <vector>
#include <string>
#include <functional>

#include "models/IShader.hpp"

namespace nx
{
  // Forward declaration
  using ShaderControlSetter = std::function<void(float)>; // normalized 0–1

  // Represents a single binding between a VST param and a Shader Control
  struct VSTParamBinding
  {
    IShader * owner { nullptr };
    int32_t vstParamID { -1 };        // 0..127
    std::string shaderControlName;  // e.g., "Brightness", "Sigma", etc.
    ShaderControlSetter setter;     // Function to call when param changes
    float lastValue { 0.f };          // last known value
  };

  class VSTParamBindingManager
  {
  public:

    VSTParamBindingManager() = default;

    void registerBindableControl( const std::string& controlName,
                                  ShaderControlSetter&& setter)
    {
      m_bindings.emplace_back(VSTParamBinding
        { nullptr, -1, controlName, setter });
    }

    void unregisterBindableControls( IShader * owner )
    {
      m_bindings.erase(
        std::remove_if( m_bindings.begin(), m_bindings.end(),
          [owner]( const VSTParamBinding& currentBinding ) -> bool
          {
            return currentBinding.owner == owner;
          }));
    }

    void unregisterBindableControl( int32_t vstParamID )
    {
      m_bindings.erase(
        std::remove_if( m_bindings.begin(), m_bindings.end(),
          [vstParamID]( const VSTParamBinding& currentBinding ) -> bool
          {
            return currentBinding.vstParamID == vstParamID;
          }));
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
      m_bindings.clear();
    }

    void setParamNormalized(const int32_t vstParamID, const float normalizedValue)
    {
      for (auto& control : m_bindings)
      {
        if (control.vstParamID == vstParamID)
        {
          if (control.setter)
          {
            control.setter(normalizedValue);
            control.lastValue = normalizedValue;
          }
        }
      }
    }

    int32_t findNextAvailableVSTParamID() const
    {
      constexpr int MaxParams = 128;
      for (int paramID = 0; paramID < MaxParams; ++paramID)
      {
        bool taken = false;
        for (const auto &control: m_bindings)
        {
          if (control.vstParamID == paramID)
          {
            taken = true;
            break;
          }
        }
        if (!taken)
          return paramID;
      }
      return -1; // No available params
    }

    [[nodiscard]]
    const auto& getBindings() const { return m_bindings; }

  private:
    std::vector<VSTParamBinding> m_bindings; // key = VST Param ID
  };
}