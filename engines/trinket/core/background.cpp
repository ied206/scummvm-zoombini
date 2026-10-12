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

#include "trinket/core/background.h"

#include "common/array.h"
#include "common/endian.h"
#include "trinket/compression/lzo1x.h"

namespace Trinket {
namespace Core {

bool Background16::decode(Common::SeekableReadStream &stream, Graphics::Surface &surface) {
	if (stream.size() < 32 || stream.readUint32LE() != 32)
		return false;
	uint32 width = stream.readUint32LE(), height = stream.readUint32LE();
	uint32 version = stream.readUint32LE(), unpacked = stream.readUint32LE(), packed = stream.readUint32LE();
	if (!width || !height || 4096 < width || 4096 < height || version != 1 || unpacked != width * height * 2 ||
		!packed || stream.size() - 32 < packed || 32 * 1024 * 1024 < packed)
		return false;
	stream.seek(32);
	Common::Array<byte> input, pixels;
	input.resize(packed);
	pixels.resize(unpacked);
	size_t decodedSize = 0;
	if (stream.read(input.data(), packed) != packed ||
		decompressLZO1X(input.data(), packed, pixels.data(), unpacked, decodedSize) != LZO1XStatus::kSuccess || decodedSize != unpacked)
		return false;
	surface.free();
	surface.create(width, height, Graphics::PixelFormat::createFormatRGBA32());
	for (uint y = 0; y < height; y++) {
		uint32 *row = static_cast<uint32 *>(surface.getBasePtr(0, y));
		for (uint x = 0; x < width; x++) {
			uint16 pixel = READ_BE_UINT16(pixels.data() + (y * width + x) * 2);
			byte red = (pixel >> 10) & 31, green = (pixel >> 5) & 31, blue = pixel & 31;
			row[x] = surface.format.RGBToColor((red << 3) | (red >> 2), (green << 3) | (green >> 2), (blue << 3) | (blue >> 2));
		}
	}
	return true;
}

} // namespace Core
} // namespace Trinket
