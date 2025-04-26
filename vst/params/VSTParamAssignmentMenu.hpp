// VSTParamAssignmentMenu.hpp
#pragma once

#include <imgui.h>
#include <string>
#include "vst/params/VSTParamBindingManager.hpp"

namespace nx
{
  inline int findNextAvailableVSTParamID(const VSTParamBindingManager &bindingManager)
  {
    constexpr int MaxParams = 128;
    for (int paramID = 0; paramID < MaxParams; ++paramID)
    {
      bool taken = false;
      for (const auto &control: bindingManager.getBindings())
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

  inline void drawVSTParamAssignmentMenu(VSTParamBindingManager &bindingManager)
  {
    if (ImGui::CollapsingHeader("VST Parameter Bindings", ImGuiTreeNodeFlags_DefaultOpen))
    {
      if (ImGui::Button("Auto-Assign All Unassigned Controls"))
      {
        for (auto &control: bindingManager.getBindings())
        {
          if (control.vstParamID == -1)
          {
            int nextID = findNextAvailableVSTParamID(bindingManager);
            if (nextID != -1)
              control.vstParamID = nextID;
          }
        }
      }

      ImGui::Spacing();
      ImGui::Separator();

      for (auto &control: bindingManager.getBindings())
      {
        ImGui::PushID(control.shaderControlName.c_str());

        std::string label = control.shaderControlName;
        if (control.vstParamID >= 0)
          label += " (Param " + std::to_string(control.assignedVSTParamID) + ")";
        else
          label += " (Unassigned)";

        ImGui::Text("%s", label.c_str());
        ImGui::SameLine();

        if (ImGui::BeginCombo("##VSTParamCombo", control.vstParamID >= 0
                                                 ? ("Param " + std::to_string(control.vstParamID)).c_str()
                                                 : "Unassigned"))
        {
          if (ImGui::Selectable("Unassigned", control.vstParamID == -1))
          {
            control.vstParamID = -1;
          }

          for (int paramID = 0; paramID < 128; ++paramID)
          {
            bool alreadyTaken = false;
            for (const auto &other: bindingManager.getBindings())
            {
              if (&other != &control && other.vstParamID == paramID)
              {
                alreadyTaken = true;
                break;
              }
            }

            if (!alreadyTaken)
            {
              std::string paramLabel = "Param " + std::to_string(paramID);
              if (ImGui::Selectable(paramLabel.c_str(), control.vstParamID == paramID))
              {
                control.vstParamID = paramID;
              }
            }
          }

          ImGui::EndCombo();
        }

        ImGui::PopID();
      }

      ImGui::Separator();
    }
  }
}