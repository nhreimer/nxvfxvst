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

#include "models/easings/TimeEasing.hpp"

#include "shapes/MidiNoteControl.hpp"
#include "shapes/TimedCursorPosition.hpp"

#include "helpers/SerialHelper.hpp"

#include "utils/LazyTexture.hpp"

#include "log/Logger.hpp"