#pragma once

#include <cmath> // std::lerp

namespace nx
{

  template <typename T>
  void drawShaderParamImGui(const char* label,
                            T& value,
                            const float min,
                            const float max,
                            const char* tooltip,
                            bool allowVSTBinding)
  {
    if constexpr (std::is_same_v<T, float>)
    {
      ImGui::SliderFloat(label, &value, min, max);
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      ImGui::Checkbox(label, &value);
    }
    else if constexpr (std::is_same_v<T, sf::Vector2f>)
    {
      ImGui::DragFloat2(label, &value.x, 0.01f);
    }
    else if constexpr (std::is_same_v<T, sf::Glsl::Vec3>)
    {
      ImGui::ColorEdit3(label, &value.x);
    }
    else if constexpr (std::is_same_v<T, sf::Color>)
    {
      ImVec4 imColor = value;
      if (ImGui::ColorEdit4(label, reinterpret_cast<float*>(&imColor)))
        value = imColor;
    }
    else if constexpr (std::is_same_v<T, sf::BlendMode>)
    {
      MenuHelper::drawBlendOptions( value );
    }

    if (tooltip && ImGui::IsItemHovered())
      ImGui::SetTooltip("%s", tooltip);
  }
}

// ========== ImGui Menu Drawing ==========
#define DRAW_FIELD(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
nx::drawShaderParamImGui<type>(#name, m_data.name, minVal, maxVal, tooltip, allowVSTBinding);
#define DRAW_FIELDS_IMGUI(PARAMS) PARAMS(DRAW_FIELD)

// =========================================
// Dispatcher Macro (must be called after binding STRUCT_REF)
// =========================================

#define DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip, allowVSTBinding) \
nx::drawShaderParamImGui<type>(#name, STRUCT_REF.name, minVal, maxVal, tooltip, allowVSTBinding);

// =========================================
// Expansion Macro
// Must call from within a drawMenu() function or similar:
//     auto& STRUCT_REF = yourStruct;
//     PARAM_MACRO(X_SHADER_IMGUI);
// =========================================

#define X_SHADER_IMGUI(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip, allowVSTBinding)

// FOR STRUCTS
#define GEN_STRUCT_FIELD(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) type name = defaultVal;
#define EXPAND_SHADER_PARAMS_FOR_STRUCT(PARAM_MACRO)     \
PARAM_MACRO(GEN_STRUCT_FIELD)                            \
/* optionally undef here if you care */                  \
/**/

// FOR ENUM
#define GEN_ENUM_FIELD(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) name,
#define EXPAND_SHADER_PARAMS_FOR_ENUM(PARAM_MACRO)       \
PARAM_MACRO(GEN_ENUM_FIELD)

// FOR IMGUI
// #define GEN_IMGUI_FLOATS(name, type, defaultVal, minVal, maxVal) \
// ImGui::SliderFloat(#name, &m_data.name, minVal, maxVal);
// #define EXPAND_SHADER_PARAMS_FOR_IMGUI(PARAM_MACRO)      \
// PARAM_MACRO(GEN_IMGUI_FLOATS)

// FOR IMGUI CONTROLS
#define GEN_TO_JSON(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
j[#name] = m_data.name;
#define EXPAND_SHADER_PARAMS_TO_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_TO_JSON)

#define GEN_FROM_JSON(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
if (j.contains(#name)) j.at(#name).get_to(m_data.name);
#define EXPAND_SHADER_PARAMS_FROM_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_FROM_JSON)

#define GEN_LABEL_STRING(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) #name,
#define EXPAND_SHADER_PARAM_LABELS(PARAM_MACRO) \
PARAM_MACRO(GEN_LABEL_STRING)

namespace nx
{
  template <typename T>
  void updateParamValue(const float normalizedValue, T& value, const float min, const float max)
  {
    if constexpr (std::is_same_v<T, float>)
    {
      value = normalizedValue * ( max - min ) + min;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      if ( normalizedValue == 0.f )
        value = false;
      else
        value = true;
    }
    else if constexpr (std::is_same_v<T, sf::Vector2f>)
    {
      // NOT SUPPORTED
    }
    else if constexpr (std::is_same_v<T, sf::Glsl::Vec3>)
    {
      // NOT SUPPORTED
    }
    else if constexpr (std::is_same_v<T, sf::Color>)
    {
      // NOT SUPPORTED
    }
    else if constexpr (std::is_same_v<T, sf::BlendMode>)
    {
      // NOT SUPPORTED

    }
  }
}

// generates our VST Parameter bindings
#define GEN_VST_BINDING(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
if constexpr (allowVSTBinding)                                                            \
{                                                                                         \
    bindingManagerRef.registerBindableControl(                                            \
        this,                                                                             \
        #name,                                                                            \
        [this](float normalizedValue) {                                                   \
            nx::updateParamValue<type>(normalizedValue, m_data.name, minVal, maxVal);     \
       });                                                                                \
}

#define EXPAND_SHADER_VST_BINDINGS(PARAM_MACRO, bindingManager) \
auto& bindingManagerRef = bindingManager; \
PARAM_MACRO(GEN_VST_BINDING)
