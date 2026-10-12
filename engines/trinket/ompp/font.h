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

#ifndef TRINKET_OMPP_FONT_H
#define TRINKET_OMPP_FONT_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "graphics/fonts/macfont.h"

namespace Trinket {
namespace OM {

/** OM endian and glyph rules on top of @ref Graphics::MacFONTFont. */
class NFNTFont : public Graphics::MacFONTFont {
public:
	/** Construct an OM strike; the caller owns the returned font. */
	static NFNTFont *load(Common::SeekableReadStream &stream);
	int getCharWidth(uint32 character) const override;
	Common::Rect getBoundingBox(uint32 character) const override;
	void drawChar(Graphics::Surface *surface, uint32 character, int x, int y, uint32 color) const override;
	int ascent() const { return getFontAscent(); }
	int descent() const { return getFontDescent(); }
	Common::Rect inkBounds(const Common::String &text) const;

private:
	using Graphics::MacFONTFont::loadFont;
	struct Metrics {
		int bearing;
		uint advance, inkWidth;
	};
	Common::Array<Metrics> _metrics;
	NFNTFont(const Graphics::MacFONTdata &data, const Common::Array<Metrics> &metrics) : Graphics::MacFONTFont(data), _metrics(metrics) {}
	NFNTFont(const NFNTFont &) = delete;
	NFNTFont &operator=(const NFNTFont &) = delete;
	uint glyph(uint32 character) const;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
