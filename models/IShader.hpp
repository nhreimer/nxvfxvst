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

#include "models/ISerializable.hpp"
#include "models/ShaderMacros.hpp"

namespace nx
{

  class VSTParamBindingManager;

  struct IShader : public ISerializable< E_ShaderType >
  {
    ~IShader() override = default;

    virtual void update( const sf::Time& deltaTime ) = 0;

    // trigger on midi event
    virtual void trigger( const Midi_t& midi ) = 0;

    // trigger on audio buffer event (can specify certain frequencies or frequency ranges)
    virtual void trigger( const AudioDataBuffer& buffer ) = 0;

    virtual void drawMenu() = 0;

    // this is used for the shutdown process
    virtual void destroyTextures() = 0;

    [[nodiscard]]
    virtual bool isShaderActive() const = 0;

    [[nodiscard]]
    virtual sf::RenderTexture * applyShader(
      const sf::RenderTexture * inputTexture ) = 0;
  };

}