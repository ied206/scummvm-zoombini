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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_COMPRESSION_NC1_H
#define TRINKET_COMPRESSION_NC1_H

#include "common/scummsys.h"

namespace Trinket {

/**
 * Decode an NC1 payload (after its 16-byte header) into disjoint caller buffers.
 * dstSize is the exact decoded size. Final runs are clipped to that size;
 * trailing source padding is ignored. Failure may leave a partial output.
 */
bool decompressNC1(const byte *src, uint32 srcSize, byte *dst, uint32 dstSize);

} // End of namespace Trinket

#endif
