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

#include "trinket/compression/nc1.h"

namespace Trinket {

bool decompressNC1(const byte *src, uint32 srcSize, byte *dst, uint32 dstSize) {
	if (!src || !dst || !srcSize || !dstSize)
		return false;

	// Every payload begins with one untagged literal byte.
	dst[0] = src[0];
	uint32 input = 1;
	uint32 output = 1;
	while (output < dstSize) {
		if (input == srcSize)
			return false;
		const byte token = src[input];
		input += 1;
		uint32 count;
		if (token & 0x80) {
			if (srcSize - input < 2)
				return false;
			const uint32 distance = ((token & 0x7f) << 8) | src[input];
			count = src[input + 1];
			input += 2;
			if (dstSize - output < count)
				count = dstSize - output;
			if (count && (!distance || output < distance))
				return false;
			// Forward copying permits a match to repeat its own output.
			for (uint32 i = 0; i < count; i++) {
				dst[output] = dst[output - distance];
				output += 1;
			}
		} else {
			count = token;
			if (dstSize - output < count)
				count = dstSize - output;
			if (srcSize - input < count)
				return false;
			memcpy(dst + output, src + input, count);
			input += count;
			output += count;
		}
	}
	return true;
}

} // namespace Trinket
