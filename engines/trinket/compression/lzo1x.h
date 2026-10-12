/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRINKET_COMPRESSION_LZO1X_H
#define TRINKET_COMPRESSION_LZO1X_H

#include "common/scummsys.h"

namespace Trinket {

// Clean-room LZO1X decoder based on the linux kernerl documentation (https://docs.kernel.org/staging/lzo.html).
// No existing LZO decoder source or disassembly was used to write this implementation.
// Validated against LZO1X streams from Trinket resources.

// FIXME: Move this decoder to common/compression and use it to replace the separate LZO1X impls in the BladeRunner and Tony engines.

enum class LZO1XStatus {
	kSuccess,
	kInputUnderrun,
	kOutputOverflow,
	kInvalidBackReference,
	kInvalidArgument,
	kTrailingInput,
	kLengthOverflow,
};

/**
 * Decode one complete raw LZO1X block, including its end marker.
 *
 * Supports the original LZO1X format, without the LZO-RLE extension.
 * The caller supplies disjoint source and destination buffers; overlapping
 * buffers are unsupported. A null buffer is valid only when its size is zero.
 * No memory is allocated. Output may be shorter than dstCapacity.
 *
 * On every return, dstSize is the number of bytes written, including on error.
 * Errors leave that output prefix intact; bytes beyond it are not modified.
 * Missing end markers and truncated operands return kInputUnderrun. Bytes after
 * an end marker return kTrailingInput. Lengths that exceed size_t return
 * kLengthOverflow. A failed copy does not write a partial literal or match run.
 */
LZO1XStatus decompressLZO1X(const byte *src, size_t srcSize, byte *dst, size_t dstCapacity, size_t &dstSize);

} // End of namespace Trinket

#endif
