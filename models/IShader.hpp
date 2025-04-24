#pragma once

#include "models/ISerializable.hpp"

// FOR STRUCTS
#define GEN_STRUCT_FIELD(name, type, defaultVal, minVal, maxVal) type name = defaultVal;
#define EXPAND_SHADER_PARAMS_FOR_STRUCT(PARAM_MACRO)     \
PARAM_MACRO(GEN_STRUCT_FIELD)                            \
/* optionally undef here if you care */                  \
/**/

// FOR ENUM
#define GEN_ENUM_FIELD(name, type, defaultVal, minVal, maxVal) name,
#define EXPAND_SHADER_PARAMS_FOR_ENUM(PARAM_MACRO)       \
PARAM_MACRO(GEN_ENUM_FIELD)

// FOR IMGUI
#define GEN_IMGUI_FLOATS(name, type, defaultVal, minVal, maxVal) \
ImGui::SliderFloat(#name, &m_data.name, minVal, maxVal);
#define EXPAND_SHADER_PARAMS_FOR_IMGUI(PARAM_MACRO)      \
PARAM_MACRO(GEN_IMGUI_FLOATS)

// FOR IMGUI CONTROLS
#define GEN_TO_JSON(name, type, defaultVal, minVal, maxVal) \
j[#name] = m_data.name;
#define EXPAND_SHADER_PARAMS_TO_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_TO_JSON)

#define GEN_FROM_JSON(name, type, defaultVal, minVal, maxVal) \
if (j.contains(#name)) j.at(#name).get_to(m_data.name);
#define EXPAND_SHADER_PARAMS_FROM_JSON(PARAM_MACRO) \
PARAM_MACRO(GEN_FROM_JSON)

#define GEN_LABEL_STRING(name, type, defaultVal, minVal, maxVal) #name,
#define EXPAND_SHADER_PARAM_LABELS(PARAM_MACRO) \
PARAM_MACRO(GEN_LABEL_STRING)

namespace nx
{

  struct IShader : public ISerializable< E_ShaderType >
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    virtual void trigger( const Midi_t& midi ) = 0;

    virtual void drawMenu() = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture& applyShader(
      const sf::RenderTexture& inputTexture ) = 0;
  };

}