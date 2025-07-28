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

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "models/easings/Easings.hpp"

namespace nx
{

#define EASING_TYPE_LIST(X)                                                                       \
X(E_Disabled,         "Disabled",         "easeDisabled",         Easings::useNone          )  \
X(E_Fixed,            "Fixed",            "easeFixed",            Easings::useNone          )  \
X(E_TimeContinuous,   "TimeContinuous",   "easeTimeContinuous",   Easings::useNone          )  \
X(E_TimeIntervallic,  "TimeIntervallic",  "easeTimeInterval",     Easings::useNone          )  \
X(E_Linear,           "Linear",           "easeLinear",           Easings::easeOutLinear    )  \
X(E_Quadratic,        "Quadratic",        "easeOutQuad",          Easings::easeOutQuad      )  \
X(E_Cubic,            "Cubic",            "easeOutCubic",         Easings::easeOutCubic     )  \
X(E_Quartic,          "Quartic",          "easeOutQuart",         Easings::easeOutQuart     )  \
X(E_Sine,             "Sine",             "easeOutSine",          Easings::easeOutSine      )  \
X(E_Expo,             "Expo",             "easeOutExpo",          Easings::easeOutExpo      )  \
X(E_Bounce,           "Bounce",           "easeOutBounce",        Easings::easeOutBounce    )  \
X(E_Back,             "Back",             "easeOutBack",          Easings::easeOutBack      )  \
X(E_SmoothPulseDecay, "SmoothPulse",      "easeSmoothDecayPulse", Easings::smoothDecayPulse )  \
X(E_Impulse,          "Impulse",          "easeImpulse",          Easings::impulse          )  \
X(E_SparkleFlicker,   "SparkleFlicker",   "easeSparkleFlicker",   Easings::sparkleFlicker   )  \
X(E_PulsePing,        "PulsePing",        "easePulsePing",        Easings::pulsePing        )  \
X(E_PulseSine,        "PulseSine",        "easePulseSine",        Easings::pulseSine        )  \
X(E_Elastic,          "Elastic",          "easeOutElastic",       Easings::easeOutElastic   )

  enum class E_EasingType : int8_t
  {
#define X(eid, label, serialLabel, func) eid,
    EASING_TYPE_LIST(X)
#undef X
    E_Count
  };

  struct EasingData_t
  {
    float decayRate { 0.1f };
    float intensity { 0.1f }; // used for certain ones

    E_EasingType easingType { E_EasingType::E_Linear };
  };

  class EasingsBase
  {

  public:

    nlohmann::json serialize() const
    {
      return
      {
        { "easingType", serializeEnum( m_data.easingType ) },
        { "decayRate", m_data.decayRate },
        { "intensity", m_data.intensity }
      };
    }

    void deserialize( const nlohmann::json& j )
    {
      m_data.easingType = deserializeEnum( j.value( "easingType", "easeLinear" ) );
      m_data.decayRate = j.value( "decayRate", 0.15f );
      m_data.intensity = j.value( "intensity", 0.1f );
    }

    EasingData_t& getData() { return m_data; }

    void setData( const EasingData_t& data )
    {
      m_data = data;
      setEasingType( m_data.easingType );
    }

    E_EasingType getEasingType() const { return m_data.easingType; }

    void setEasingType(const E_EasingType easingType)
    {
      switch (easingType)
      {
#define X(eid, label, serialLabel, func) case E_EasingType::eid: m_easingFunction = func; break;
        EASING_TYPE_LIST(X)
#undef X
        default:
          m_easingFunction = Easings::easeOutLinear;
        break;
      }
    }

    float getDecayRate() const { return m_data.decayRate; }

    void drawMenu()
    {
      ImGui::PushID(this);
      ImGui::Text("Time Easing Functions:");

      const int32_t currentIndex = toComboIndex(m_data.easingType);
      const char* currentLabel = m_easingFunctionNames[currentIndex];

      if (ImGui::BeginCombo("##Easings", currentLabel))
      {
#define X(eid, label, func)                                                        \
{                                                                                  \
const int32_t idx = static_cast<int32_t>(E_EasingType::eid);                       \
const bool isSelected = (currentIndex == idx);                                     \
if (ImGui::Selectable(label, isSelected))                                          \
  m_data.easingType = E_EasingType::eid;                                           \
if (isSelected) ImGui::SetItemDefaultFocus();                                      \
}
        EASING_TYPE_LIST(X)
#undef X

        ImGui::EndCombo();
      }

      // Show easing-specific sliders
      drawParamsForEasingType();

      ImGui::PopID();
    }

  protected:
    EasingData_t m_data;
    std::function< float( float t ) > m_easingFunction { Easings::useNone };
    int32_t m_selectedComboIndex { 4 };

  private:

    void drawParamsForEasingType()
    {
      if (m_data.easingType == E_EasingType::E_Disabled)
        return;

      ImGui::SliderFloat("##DecayRate", &m_data.decayRate, 0.01f, 1.5f, "Decay Rate %.2f seconds");

      switch (m_data.easingType)
      {
        case E_EasingType::E_SparkleFlicker:
          ImGui::SliderFloat("##Sparkle Intensity", &m_data.intensity, 0.f, 3.f, "Intensity %.2f");
          break;
        case E_EasingType::E_Impulse:
        case E_EasingType::E_Fixed:
          ImGui::SliderFloat("##Impulse Scale", &m_data.intensity, 0.f, 10.f, "Scale %.2f");
          break;
        default:
          break;
      }
    }

    static int32_t toComboIndex(const E_EasingType type)
    {
      return static_cast<int32_t>(type); // guaranteed to match list order via macro
    }

    static E_EasingType fromComboIndex(const int32_t index)
    {
      return static_cast<E_EasingType>(index);
    }

    inline static const std::array<const char*, static_cast<size_t>(E_EasingType::E_Count)> m_easingFunctionNames =
    {
#define X(eid, label, serialLabel, func) label,
      EASING_TYPE_LIST(X)
#undef X
    };

    inline static const std::array<E_EasingType, static_cast<size_t>(E_EasingType::E_Count)> m_easingTypes =
    {
#define X(eid, label, serialLabel, func) E_EasingType::eid,
      EASING_TYPE_LIST(X)
#undef X
    };

    inline static std::array< const char *, static_cast< size_t >(E_EasingType::E_Count) > kEasingTypeToString = {
#define X(eid, label, serialLabel, func) serialLabel,
      EASING_TYPE_LIST(X)
#undef X
    };

    static constexpr const char* serializeEnum(E_EasingType e)
    {
      return kEasingTypeToString[static_cast<size_t>(e)];
    }

    inline static const std::unordered_map<std::string_view, E_EasingType> kStringToEasingType = {
#define X(eid, label, serialLabel, func) { serialLabel, E_EasingType::eid },
      EASING_TYPE_LIST(X)
#undef X
    };

    static E_EasingType deserializeEnum(const std::string_view s)
    {
      const auto it = kStringToEasingType.find(s);
      if (it != kStringToEasingType.end())
        return it->second;

      return E_EasingType::E_Linear;
    }

    NLOHMANN_JSON_SERIALIZE_ENUM(E_EasingType,
    {
      { E_EasingType::E_Disabled, "Disabled" },
      { E_EasingType::E_Fixed, "Fixed" },
      { E_EasingType::E_TimeContinuous, "TimeContinuous" },
      { E_EasingType::E_TimeIntervallic, "TimeIntervallic" },
      { E_EasingType::E_Linear, "Linear" },
      { E_EasingType::E_Quadratic, "Quadratic" },
      { E_EasingType::E_Cubic, "Cubic" },
      { E_EasingType::E_Quartic, "Quartic" },
      { E_EasingType::E_Sine, "Sine" },
      { E_EasingType::E_Expo, "Expo" },
      { E_EasingType::E_Bounce, "Bounce" },
      { E_EasingType::E_Back, "Back" },
      { E_EasingType::E_SmoothPulseDecay, "SmoothPulse" },
      { E_EasingType::E_Impulse, "Impulse" },
      { E_EasingType::E_SparkleFlicker, "SparkleFlicker" },
      { E_EasingType::E_PulsePing, "PulsePing" },
      { E_EasingType::E_PulseSine, "PulseSine" },
      { E_EasingType::E_Elastic, "Elastic" }
    })
  };
}