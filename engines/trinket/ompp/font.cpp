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

#include "trinket/ompp/font.h"

#include "common/endian.h"
#include "graphics/surface.h"

namespace Trinket {
namespace OM {

NFNTFont *NFNTFont::load(Common::SeekableReadStream &stream) {
	byte raw[26];
	if (stream.read(raw, sizeof(raw)) != sizeof(raw))
		return nullptr;
	bool littleEndian = (READ_LE_UINT16(raw) & 32768) != 0;
	uint16 header[13];
	for (uint i = 0; i < ARRAYSIZE(header); i++)
		header[i] = littleEndian ? READ_LE_UINT16(raw + i * 2) : READ_BE_UINT16(raw + i * 2);
	uint first = header[1];
	uint last = header[2];
	if (first != 0 || last < 1 || 255 < last)
		return nullptr;
	uint rows = header[7], rowBytes = header[12] * 2;
	uint count = last - first + 3;
	uint32 metricsOffset = 16 + header[8] * 2;
	uint64 bitmapBytes = static_cast<uint64>(rowBytes) * rows;
	if (!bitmapBytes || metricsOffset < 26 + bitmapBytes + count * 2 ||
		stream.size() < metricsOffset + count * 2 || rows < static_cast<uint>(header[9] + header[10]))
		return nullptr;
	Common::ScopedPtr<byte, Common::ArrayDeleter<byte>> bitmap(new byte[static_cast<uint>(bitmapBytes)]);
	if (stream.read(bitmap.get(), static_cast<uint>(bitmapBytes)) != bitmapBytes || !stream.seek(metricsOffset - count * 2))
		return nullptr;
	Common::Array<uint16> locations;
	locations.resize(count);
	for (uint i = 0; i < count; i++) {
		locations[i] = littleEndian ? stream.readUint16LE() : stream.readUint16BE();
		// OM copies a spare word after the final glyph boundary; it is not a location.
		if (i <= last + 1 && (rowBytes * 8 < locations[i] || (i && locations[i] < locations[i - 1])))
			return nullptr;
	}
	Common::Array<byte> offsets;
	offsets.resize(count * 2);
	if (stream.read(offsets.data(), offsets.size()) != offsets.size() || stream.err())
		return nullptr;
	Graphics::MacFONTdata data = {};
	data._fontType = header[0];
	data._firstChar = first;
	data._lastChar = last;
	data._maxWidth = header[3];
	data._kernMax = static_cast<int16>(header[4]);
	data._fRectWidth = header[6];
	data._fRectHeight = header[9] + header[10];
	data._ascent = header[9];
	data._descent = header[10];
	data._leading = header[11];
	data._rowWords = rowBytes;
	data._surfHeight = rows;
	data._size = 12;
	data._glyphs.resize(last + 1);
	Common::Array<Metrics> metrics;
	metrics.resize(last + 1);
	for (uint i = 0; i <= last; i++) {
		uint source = offsets[i * 2] == 255 && offsets[i * 2 + 1] == 255 ? (last - 1) & 255 : i;
		Metrics &metric = metrics[i];
		metric.bearing = data._kernMax + offsets[source * 2];
		metric.advance = offsets[source * 2 + 1];
		metric.inkWidth = locations[source + 1] - locations[source];
		Graphics::MacGlyph &glyph = data._glyphs[i];
		glyph.clear();
		glyph.bitmapOffset = locations[source];
		glyph.bitmapWidth = metric.inkWidth;
		// MacFONTFont skips zero-advance glyphs; OM still draws their strike.
		glyph.width = MAX<uint>(1, metric.advance);
	}
	data._defaultChar = data._glyphs[(last - 1) & 255];
	// MacFONTFont owns the strike. Keep OM's endian, fallback and height rules here.
	data._bitImage = bitmap.release();
	return new NFNTFont(data, metrics);
}

uint NFNTFont::glyph(uint32 character) const {
	return character < _metrics.size() ? character : (_metrics.size() - 2) & 255;
}

int NFNTFont::getCharWidth(uint32 character) const {
	return _metrics[glyph(character)].advance;
}

Common::Rect NFNTFont::getBoundingBox(uint32 character) const {
	const Metrics &metric = _metrics[glyph(character)];
	// Shared string culling must use the strike bounds, including bearing and overhang.
	return Common::Rect(metric.bearing, 0, metric.bearing + metric.inkWidth, ascent() + descent());
}

Common::Rect NFNTFont::inkBounds(const Common::String &text) const {
	if (text.empty())
		return Common::Rect(0, -ascent(), 0, descent());
	uint first = glyph(static_cast<byte>(text.firstChar()));
	uint last = glyph(static_cast<byte>(text.lastChar()));
	int left = _metrics[first].bearing;
	int right = getStringWidth(text) + _metrics[last].bearing + _metrics[last].inkWidth - _metrics[last].advance;
	return Common::Rect(left, -ascent(), right, descent());
}

void NFNTFont::drawChar(Graphics::Surface *surface, uint32 character, int x, int y, uint32 color) const {
	uint index = glyph(character);
	// Apply the OM bearing before MacFONTFont's clipping tests.
	Graphics::MacFONTFont::drawChar(surface, index, x + _metrics[index].bearing, y, color);
}

} // namespace OM
} // namespace Trinket
