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

#include "trinket/compression/lzo1x.h"

namespace Trinket {

LZO1XStatus decompressLZO1X(const byte *src, size_t srcSize, byte *dst, size_t dstCapacity, size_t &dstSize) {
	dstSize = 0;
	if ((!src && srcSize != 0) || (!dst && dstCapacity != 0))
		return LZO1XStatus::kInvalidArgument;

	size_t srcPos = 0;
	// Four denotes a standalone literal run; smaller values count match trailers.
	unsigned literalState = 0;
	while (true) {
		if (srcPos == srcSize)
			return LZO1XStatus::kInputUnderrun;
		const unsigned token = src[srcPos];
		srcPos += 1;

		bool literalRun = false;
		size_t length = 0;
		size_t distance = 0;
		unsigned trailingLiterals = 0;
		unsigned lengthMask = 0;
		unsigned lengthBias = 0;
		if (srcPos == 1 && 17 < token) {
			literalRun = true;
			length = token - 17;
		} else if (token < 16 && literalState == 0) {
			literalRun = true;
			lengthMask = 15;
			lengthBias = 3;
		} else if (token < 16 || 64 <= token) {
			if (srcPos == srcSize)
				return LZO1XStatus::kInputUnderrun;
			const unsigned operand = src[srcPos];
			srcPos += 1;
			trailingLiterals = token & 3;
			if (token < 16) {
				length = literalState == 4 ? 3 : 2;
				distance = (token >> 2) + (operand << 2) + 1;
				if (literalState == 4)
					distance += 2048;
			} else {
				length = (token >> 5) + 1;
				distance = ((token >> 2) & 7) + (operand << 3) + 1;
			}
		} else {
			lengthMask = token < 32 ? 7 : 31;
			lengthBias = 2;
		}

		if (lengthMask != 0) {
			length = token & lengthMask;
			if (length == 0) {
				length = lengthMask;
				unsigned extension = 0;
				do {
					if (srcPos == srcSize)
						return LZO1XStatus::kInputUnderrun;
					extension = src[srcPos];
					srcPos += 1;
					const size_t increment = extension == 0 ? 255 : extension;
					if (static_cast<size_t>(-1) - length < increment)
						return LZO1XStatus::kLengthOverflow;
					length += increment;
				} while (extension == 0);
			}
			if (static_cast<size_t>(-1) - length < lengthBias)
				return LZO1XStatus::kLengthOverflow;
			length += lengthBias;
		}

		if (literalRun) {
			if (srcSize - srcPos < length)
				return LZO1XStatus::kInputUnderrun;
			if (dstCapacity - dstSize < length)
				return LZO1XStatus::kOutputOverflow;
			for (size_t i = 0; i < length; i++)
				dst[dstSize + i] = src[srcPos + i];
			srcPos += length;
			dstSize += length;
			literalState = length < 4 ? static_cast<unsigned>(length) : 4;
			continue;
		}

		if (16 <= token && token < 64) {
			if (srcSize - srcPos < 2)
				return LZO1XStatus::kInputUnderrun;
			const unsigned operand = src[srcPos] | (static_cast<unsigned>(src[srcPos + 1]) << 8);
			srcPos += 2;
			trailingLiterals = operand & 3;
			distance = operand >> 2;
			if (token < 32) {
				distance += (token & 8) << 11;
				if (distance == 0)
					return srcPos == srcSize ? LZO1XStatus::kSuccess : LZO1XStatus::kTrailingInput;
				distance += 16384;
			} else {
				distance += 1;
			}
		}

		if (dstSize < distance)
			return LZO1XStatus::kInvalidBackReference;
		if (dstCapacity - dstSize < length)
			return LZO1XStatus::kOutputOverflow;
		// Forward copying allows a match to reuse bytes produced by that match.
		for (size_t i = 0; i < length; i++)
			dst[dstSize + i] = dst[dstSize - distance + i];
		dstSize += length;

		if (srcSize - srcPos < trailingLiterals)
			return LZO1XStatus::kInputUnderrun;
		if (dstCapacity - dstSize < trailingLiterals)
			return LZO1XStatus::kOutputOverflow;
		for (unsigned i = 0; i < trailingLiterals; i++)
			dst[dstSize + i] = src[srcPos + i];
		srcPos += trailingLiterals;
		dstSize += trailingLiterals;
		literalState = trailingLiterals;
	}
}

} // End of namespace Trinket
