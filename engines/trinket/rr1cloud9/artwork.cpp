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

#include "trinket/rr1cloud9/artwork.h"

#include "common/ptr.h"
#include "common/str.h"

namespace Trinket {
namespace RR1Cloud9 {

void Artwork::clear() {
	for (auto &frame : _frames) {
		frame.pixels.clear();
		frame.marked = false;
	}
}

int Artwork::markedCount() const {
	int count = 0;
	for (const auto &frame : _frames)
		count += frame.marked;
	return count;
}

bool Artwork::image(const OM::ResourceProvider &resources, int book, int page, Common::Array<uint16> &pixels) const {
	pixels = frame(book, page).pixels;
	if (!pixels.empty())
		return true;
	pixels.resize(kWidth * kHeight, kWhite);
	if (3 < book || (book == 3 && 14 < page))
		return true;
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources.open(Common::String::format("samples/SampleB%dI%02d.DAT", book, page)));
	if (!stream || stream->size() != 10 + kWidth * kHeight * 2)
		return false;
	uint version = stream->readUint16LE();
	bool big = version == 256;
	if (version != 1 && !big)
		return false;
	uint size = big ? stream->readUint32BE() : stream->readUint32LE();
	uint width = big ? stream->readUint16BE() : stream->readUint16LE();
	uint height = big ? stream->readUint16BE() : stream->readUint16LE();
	if (size != 4 + kWidth * kHeight * 2 || width != kWidth || height != kHeight)
		return false;
	for (auto &pixel : pixels)
		pixel = (big ? stream->readUint16BE() : stream->readUint16LE()) & kWhite;
	return !stream->err() && !stream->eos();
}

bool Artwork::save(Common::WriteStream &stream) const {
	for (const auto &frame : _frames) {
		stream.writeByte((frame.marked ? 1 : 0) | (frame.pixels.empty() ? 0 : 2));
		for (uint16 pixel : frame.pixels)
			stream.writeUint16LE(pixel);
	}
	return !stream.err();
}

bool Artwork::load(Common::ReadStream &stream) {
	Artwork pending;
	for (auto &frame : pending._frames) {
		uint flags = stream.readByte();
		if (3 < flags || stream.err() || stream.eos())
			return false;
		frame.marked = (flags & 1) != 0;
		if (flags & 2) {
			frame.pixels.resize(kWidth * kHeight);
			for (auto &pixel : frame.pixels) {
				pixel = stream.readUint16LE();
				if (kWhite < pixel || stream.err() || stream.eos())
					return false;
			}
		}
	}
	*this = pending;
	return true;
}

void Artwork::render(const Common::Array<uint16> &pixels, Graphics::Surface &surface, bool preview) {
	surface.free();
	surface.create(preview ? 151 : kWidth, preview ? 100 : kVisibleHeight, Graphics::PixelFormat::createFormatRGBA32());
	for (int y = 0; y < kHeight; y++) {
		int targetY = preview ? y * 102 / kHeight : y;
		if (surface.h <= targetY)
			continue;
		for (int x = 0; x < kWidth; x++) {
			uint c = pixels[y * kWidth + x], r = (c >> 10) & 31, g = (c >> 5) & 31, b = c & 31;
			surface.setPixel(preview ? x * 151 / kWidth : x, targetY,
							 surface.format.RGBToColor((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2)));
		}
	}
}
} // namespace RR1Cloud9
} // namespace Trinket
