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

namespace nx
{

  struct IFFTResult
  {
    virtual ~IFFTResult() = default;
    virtual const AudioDataBuffer& getSmoothedBuffer() const = 0;
    virtual const AudioDataBuffer& getRealTimeBuffer() const = 0;

    // virtual const AudioDataBuffer& getLogSmoothedBuffer() const = 0;
    // virtual const AudioDataBuffer& getLogRealTimeBuffer() const = 0;

    // Optional: Musical or note-mapped buffers?
    // virtual const std::vector<NoteEnergy>& getNoteMappedBuffer() const = 0;
  };

}