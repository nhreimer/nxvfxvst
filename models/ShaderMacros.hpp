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

#include "log/Logger.hpp"
#include "helpers/MenuHelper.hpp"
#include "models/easings/EasingsBase.hpp"

/******************************************************************************
 * this is poorly named and needs to be updated since its role has expanded to
 * include all components that have user controls.
 *
 * the following macros and  functions are used to automate VSTParameter binding,
 * unbinding, JSON serialization, ImGui controls, and a few other features.
 * they're not strictly necessary, but they significantly improve the quality of
 * life at the expense of macro usage and all the perils that come with them.
 ******************************************************************************/
namespace nx
{

  template <typename T>
  void drawShaderParamImGui(const char* label,
                            T& value,
                            const int32_t paramId,
                            const float min,
                            const float max,
                            const char* tooltip,
                            const bool allowVSTBinding)
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
    else if constexpr (std::is_same_v<T, int32_t>)
    {
      ImGui::SliderInt(label, &value, min, max);
    }
    else
    {
      LOG_ERROR( "Unsupported ImGui Data Type: {}", label );
    }

    if ( allowVSTBinding )
    {
      ImGui::SameLine();
      ImGui::Text( " (%d)", paramId );
    }

    if (tooltip && ImGui::IsItemHovered())
      ImGui::SetTooltip("%s", tooltip);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

// ========== ImGui Menu Drawing ===========

#define DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip, allowVSTBinding) \
nx::drawShaderParamImGui<type>(#name, STRUCT_REF.name.first, STRUCT_REF.name.second, minVal, maxVal, tooltip, allowVSTBinding);

#define X_SHADER_IMGUI(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
DISPATCH_IMGUI_FIELD(name, type, STRUCT_REF, minVal, maxVal, tooltip, allowVSTBinding)

#define EXPAND_SHADER_IMGUI(PARAM_MACRO, DATA_STRUCT) \
auto& STRUCT_REF = DATA_STRUCT;                       \
PARAM_MACRO(X_SHADER_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////

// FOR STRUCTS
#define GEN_STRUCT_FIELD(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
std::pair<type, int32_t> name = { defaultVal, -1 };

#define EXPAND_SHADER_PARAMS_FOR_STRUCT(PARAM_MACRO)     \
PARAM_MACRO(GEN_STRUCT_FIELD)

////////////////////////////////////////////////////////////////////////////////////////////

// FOR ENUM
#define GEN_ENUM_FIELD(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) name,
#define EXPAND_SHADER_PARAMS_FOR_ENUM(PARAM_MACRO)       \
PARAM_MACRO(GEN_ENUM_FIELD)

////////////////////////////////////////////////////////////////////////////////////////////

// FOR SERIALIZATION
#define GEN_TO_JSON(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
j[#name] = m_data.name.first;
#define EXPAND_SHADER_PARAMS_TO_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_TO_JSON)

#define GEN_FROM_JSON(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
if (j.contains(#name)) j.at(#name).get_to(m_data.name.first);
#define EXPAND_SHADER_PARAMS_FROM_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_FROM_JSON)

#define GEN_LABEL_STRING(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) #name,
#define EXPAND_SHADER_PARAM_LABELS(PARAM_MACRO) \
PARAM_MACRO(GEN_LABEL_STRING)

////////////////////////////////////////////////////////////////////////////////////////////

namespace nx
{
  template <typename T>
  float updateParamValue(const float normalizedValue, T& value, const float min, const float max)
  {
    if constexpr (std::is_same_v<T, float>)
    {
      value = normalizedValue * ( max - min ) + min;
      return value;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      value = ( normalizedValue > 0.f ? 1.f : 0.f );
      return value;
    }
    else if constexpr (std::is_same_v<T, sf::Vector2f>)
    {
      // NOT SUPPORTED
      return 0.f;
    }
    else if constexpr (std::is_same_v<T, sf::Glsl::Vec3>)
    {
      // NOT SUPPORTED
      return 0.f;
    }
    else if constexpr (std::is_same_v<T, sf::Color>)
    {
      // NOT SUPPORTED
      return 0.f;
    }
    else if constexpr (std::is_same_v<T, sf::BlendMode>)
    {
      // NOT SUPPORTED
      return 0.f;
    }
    else if constexpr (std::is_same_v<T, E_EasingType>)
    {
      // NOT SUPPORTED
      return 0.f;
    }
    return 0.f;
  }
}

// generates our VST Parameter bindings
#define GEN_VST_BINDING(name, type, defaultVal, minVal, maxVal, tooltip, allowVSTBinding) \
if constexpr (allowVSTBinding)                                                            \
{                                                                                         \
    auto paramId = bindingManagerRef.registerBindableControl(                             \
        this,                                                                             \
        #name,                                                                            \
        minVal,                                                                           \
        maxVal,                                                                           \
        [this](float normalizedValue) {                                                   \
           return nx::updateParamValue<type>(normalizedValue, m_data.name.first, minVal, maxVal); \
       });                                                                                \
     m_data.name.second = paramId;                                                        \
     bindingManagerRef.setValue<type>(paramId, m_data.name.first);                        \
}

#define EXPAND_SHADER_VST_BINDINGS(PARAM_MACRO, bindingManager) \
auto& bindingManagerRef = bindingManager; \
PARAM_MACRO(GEN_VST_BINDING)
