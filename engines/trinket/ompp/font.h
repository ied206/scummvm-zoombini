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
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "graphics/fonts/macfont.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace OM {

/** OM endian and glyph rules on top of @ref Graphics::MacFONTFont. */
class NFNTFont : public Graphics::MacFONTFont {
public:
	/** Decode an OM font strike, returning null on failure; the caller must delete a successful result. */
	static NFNTFont *load(Common::SeekableReadStream &stream);
	/** Return the glyph's horizontal pen advance in pixels, using the fallback glyph when needed. */
	int getCharWidth(uint32 character) const override;
	/** Return the glyph's ink bounds relative to the top of the font strike. */
	Common::Rect getBoundingBox(uint32 character) const override;
	/** Draw a glyph into the surface at its bearing-adjusted pixel position. */
	void drawChar(Graphics::Surface *surface, uint32 character, int x, int y, uint32 color) const override;
	/** Return the strike height in pixels above the text baseline. */
	int ascent() const { return getFontAscent(); }
	/** Return the strike height in pixels below the text baseline. */
	int descent() const { return getFontDescent(); }
	/** Return a byte string's ink bounds relative to its baseline. */
	Common::Rect inkBounds(const Common::String &text) const;

private:
	using Graphics::MacFONTFont::loadFont;
	struct Metrics {
		/** Horizontal distance in pixels from the pen position to the glyph's ink. */
		int bearing;
		/** Horizontal pen advance in pixels after drawing the glyph. */
		uint advance;
		/** Width in pixels of the glyph's bitmap strike. */
		uint inkWidth;
	};
	/** Glyph-indexed advances, bearings and strike widths retained with the font. */
	Common::Array<Metrics> _metrics;
	/** Initialize the decoded font strike and its character metrics. */
	NFNTFont(const Graphics::MacFONTdata &data, const Common::Array<Metrics> &metrics) : Graphics::MacFONTFont(data), _metrics(metrics) {}
	/** Prevent accidental copies of this stateful object. */
	NFNTFont(const NFNTFont &) = delete;
	/** Prevent accidental copies of this stateful object. */
	NFNTFont &operator=(const NFNTFont &) = delete;
	/** Map a character code to a valid glyph index, falling back for unsupported codes. */
	uint glyph(uint32 character) const;
};

/** Share immutable NFNT strikes within one game's or launcher's resource namespace. */
class FontCache {
public:
	/** Create a font cache for the supplied resource provider. */
	explicit FontCache(const ResourceProvider &resources) : _resources(resources) {}
	/** Load a strike once; callers retain it independently of cache eviction. */
	Common::SharedPtr<NFNTFont> load(const Common::String &path);
	/** Remove strikes no longer referenced outside the cache. */
	void removeUnused();
	/** Release all cache references; active callers keep their strikes alive. */
	void clear() { _fonts.clear(); }
	/** Return the number of retained strikes. */
	uint size() const { return _fonts.size(); }
	/** Return cumulative successful strike decodes. */
	uint64 decodes() const { return _decodes; }
	/** Return cumulative requests satisfied by existing strikes. */
	uint64 hits() const { return _hits; }

private:
	/** Resource provider used to load font strikes within this cache's namespace. */
	const ResourceProvider &_resources;
	/** Normalized provider identities mapped to shared immutable strikes. */
	Common::HashMap<Common::String, Common::SharedPtr<NFNTFont>> _fonts;
	/** Successful decodes over this cache instance's lifetime. */
	uint64 _decodes = 0;
	/** Cache hits over this cache instance's lifetime. */
	uint64 _hits = 0;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
