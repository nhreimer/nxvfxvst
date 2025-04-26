#pragma once

#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <functional>

namespace nx
{
  // Forward declaration
  using ShaderControlSetter = std::function<void(float)>; // normalized 0–1

  // Represents a single binding between a VST param and a Shader Control
  struct VSTParamBinding
  {
    int32_t vstParamID = -1;        // 0..127
    std::string shaderControlName;  // e.g., "Brightness", "Sigma", etc.
    ShaderControlSetter setter;     // Function to call when param changes
    float lastValue = 0.f;          // last known value
  };

  class VSTParamBindingManager
  {
  public:

    VSTParamBindingManager() = default;

    void registerBindableControl(const std::string& controlName, ShaderControlSetter&& setter)
    {
      m_bindings.emplace_back(VSTParamBinding { -1, controlName, setter });
    }

    void assignParamIDToControl(const std::string& controlName, const int32_t vstParamID)
    {
      for (auto& control : m_bindings)
      {
        if (control.shaderControlName == controlName)
        {
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


    [[nodiscard]]
    const auto& getBindings() const { return m_bindings; }

  private:
    std::vector<VSTParamBinding> m_bindings; // key = VST Param ID
  };
}