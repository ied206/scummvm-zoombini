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

/** Load NFNT strikes into FontMan for one game's or launcher's resource lifetime. */
class FontCache {
public:
	/** Create an isolated FontMan namespace for the supplied resource provider. */
	explicit FontCache(const ResourceProvider &resources) : _resources(resources) {}
	/** Remove this namespace's FontMan registrations after all font consumers have been destroyed. */
	~FontCache() { clear(); }
	/** Return a FontMan-owned strike valid until clear() or destruction, or null on load failure. */
	const Graphics::Font *load(const Common::String &path);
	/** Remove all registered names, invalidating borrowed fonts without other FontMan aliases. */
	void clear();
	/** Return the number of strikes registered by this loader. */
	uint size() const { return _fontNames.size(); }
	/** Return cumulative successful strike decodes. */
	uint64 decodes() const { return _decodes; }
	/** Return cumulative requests satisfied by existing strikes. */
	uint64 hits() const { return _hits; }

private:
	/** Resource provider used to load font strikes within this cache's namespace. */
	const ResourceProvider &_resources;
	/** FontMan names registered by this loader; FontMan owns the corresponding strikes. */
	Common::Array<Common::String> _fontNames;
	/** Successful decodes over this cache instance's lifetime. */
	uint64 _decodes = 0;
	/** Cache hits over this cache instance's lifetime. */
	uint64 _hits = 0;
	/** Prevent copying the namespace and its registration cleanup responsibility. */
	FontCache(const FontCache &) = delete;
	/** Prevent replacing the namespace and its registration cleanup responsibility. */
	FontCache &operator=(const FontCache &) = delete;
	/** Qualify a resolved provider key with this loader's identity to isolate simultaneous resource sets. */
	Common::String cacheName(const Common::String &key) const;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
