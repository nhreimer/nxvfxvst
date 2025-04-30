#pragma once

#include <deque>
#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics.hpp>

#include "models/data/GlobalInfo_t.hpp"
#include "models/data/ParticleLayoutData_t.hpp"
#include "models/data/TimedParticle_t.hpp"
#include "models/data/Midi_t.hpp"
#include "models/data/PipelineContext.hpp"

#include "models/InterfaceTypes.hpp"
#include "models/IShader.hpp"
#include "models/IParticleLayout.hpp"
#include "models/IParticleModifier.hpp"

#include "models/shader/easings/TimeEasing.hpp"

#include "shapes/MidiNoteControl.hpp"
#include "shapes/TimedCursorPosition.hpp"

#include "helpers/SerialHelper.hpp"

#include "log/Logger.hpp"