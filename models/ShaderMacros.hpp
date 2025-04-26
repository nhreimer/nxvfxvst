#pragma once


namespace nx
{
  template <typename T>
  void drawShaderParamImGui(const char* label, T& value, float min, float max, const char* tooltip)
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
#define DRAW_FIELD(name, type, defaultVal, minVal, maxVal, tooltip) \
nx::drawShaderParamImGui<type>(#name, m_data.name, minVal, maxVal, tooltip);
#define DRAW_FIELDS_IMGUI(PARAMS) PARAMS(DRAW_FIELD)

// =========================================
// Dispatcher Macro (must be called after binding STRUCT_REF)
// =========================================

#define DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip) \
nx::drawShaderParamImGui<type>(#name, STRUCT_REF.name, minVal, maxVal, tooltip);

// =========================================
// Expansion Macro
// Must call from within a drawMenu() function or similar:
//     auto& STRUCT_REF = yourStruct;
//     PARAM_MACRO(X_SHADER_IMGUI);
// =========================================

#define X_SHADER_IMGUI(name, type, defaultVal, minVal, maxVal, tooltip) \
DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip)


// FOR STRUCTS
#define GEN_STRUCT_FIELD(name, type, defaultVal, minVal, maxVal, tooltip) type name = defaultVal;
#define EXPAND_SHADER_PARAMS_FOR_STRUCT(PARAM_MACRO)     \
PARAM_MACRO(GEN_STRUCT_FIELD)                            \
/* optionally undef here if you care */                  \
/**/

// FOR ENUM
#define GEN_ENUM_FIELD(name, type, defaultVal, minVal, maxVal, tooltip) name,
#define EXPAND_SHADER_PARAMS_FOR_ENUM(PARAM_MACRO)       \
PARAM_MACRO(GEN_ENUM_FIELD)

// FOR IMGUI
#define GEN_IMGUI_FLOATS(name, type, defaultVal, minVal, maxVal) \
ImGui::SliderFloat(#name, &m_data.name, minVal, maxVal);
#define EXPAND_SHADER_PARAMS_FOR_IMGUI(PARAM_MACRO)      \
PARAM_MACRO(GEN_IMGUI_FLOATS)

// FOR IMGUI CONTROLS
#define GEN_TO_JSON(name, type, defaultVal, minVal, maxVal, tooltip) \
j[#name] = m_data.name;
#define EXPAND_SHADER_PARAMS_TO_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_TO_JSON)

#define GEN_FROM_JSON(name, type, defaultVal, minVal, maxVal, tooltip) \
if (j.contains(#name)) j.at(#name).get_to(m_data.name);
#define EXPAND_SHADER_PARAMS_FROM_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_FROM_JSON)

#define GEN_LABEL_STRING(name, type, defaultVal, minVal, maxVal, tooltip) #name,
#define EXPAND_SHADER_PARAM_LABELS(PARAM_MACRO) \
PARAM_MACRO(GEN_LABEL_STRING)

namespace nx
{
  template <typename T>
  void registerParamControl(const char* label, T& value, const float min, const float max)
  {
    if constexpr (std::is_same_v<T, float>)
    {
      value = std::lerp( min, max, value );
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      value = !value;
    }
    else if constexpr (std::is_same_v<T, sf::Vector2f>)
    {
    }
    else if constexpr (std::is_same_v<T, sf::Glsl::Vec3>)
    {
    }
    else if constexpr (std::is_same_v<T, sf::Color>)
    {
    }
    else if constexpr (std::is_same_v<T, sf::BlendMode>)
    {
    }
  }
}

// generates our VST Parameter bindings
#define GEN_VST_BINDING(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
if constexpr (allowVSTBinding)                                                            \
{                                                                                         \
    bindingManager.registerBindableControl(                                               \
        #name,                                                                            \
        [this](float normalizedValue) {                                                   \
            nx::registerParamControl<type>(#name, m_data.name, minVal, maxVal);           \
        });                                                                               \
}


