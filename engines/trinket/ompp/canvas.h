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

#ifndef TRINKET_OMPP_CANVAS_H
#define TRINKET_OMPP_CANVAS_H

#include "common/array.h"
#include "graphics/surface.h"

namespace Trinket {
namespace OM {

class Canvas {
public:
	/**
	 * Display-only color conversions for selected puzzle sprites, independent of authored trait codes.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 * Callers select the authored color category; these conversions do not detect source hues.
	 */
	enum class ColorRemap {
		kNone,                        ///< Preserve original pixels.
		kGreenToCyan,                 ///< Replace green with cyan while retaining shading.
		kPurpleOrMagentaToDarkPurple, ///< Replace purple or magenta with dark purple while retaining shading.
		kGreenToGray,                 ///< Replace green with neutral gray, lifting midtones and retaining shading.
		kOrangeToWhite,               ///< Replace orange with bright neutral tones while retaining shading.
		kOrangeToDarkGray,            ///< Replace orange with darker neutral midtones while retaining bright highlights.
	};
	/**
	 * Optional source-color restriction for a recolored image region.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	enum class ColorSource {
		kAny,           ///< Accept every chromatic source color.
		kRedDominant,   ///< Accept pixels whose red channel is strictly largest.
		kGreenDominant, ///< Accept pixels whose green channel is strictly largest.
	};
	/**
	 * A source-local recolor region within a multicolored image.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	struct ColorRemapRegion {
		/** Select source pixel bounds, color conversion and optional source-channel restriction. */
		ColorRemapRegion(const Common::Rect &area, ColorRemap target, ColorSource filter = ColorSource::kAny)
			: bounds(area), remap(target), source(filter) {}
		Common::Rect bounds; ///< Half-open bounds in unmirrored source pixels.
		ColorRemap remap;    ///< Color conversion applied inside the matching region.
		ColorSource source;  ///< Source-channel restriction that preserves neighboring artwork.
	};
	/** Create an opaque-composition canvas with original colors. */
	Canvas(int width, int height);
	/** Free the canvas pixel buffer. */
	~Canvas();
	/** Return the canvas surface for drawing. */
	Graphics::Surface &surface() { return _surface; }
	/** Return the canvas surface for reading. */
	const Graphics::Surface &surface() const { return _surface; }
	/**
	 * Enable display-only recoloring of explicitly marked puzzle sprites.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	void setColorBlindMode(bool enabled) { _colorBlindMode = enabled; }
	/**
	 * Composite source pixels; the first matching region overrides the whole-image remap before blending.
	 * @note Color remaps and regions are ScummVM additions absent from the original engine.
	 */
	void blit(const Graphics::Surface &source, int x, int y, bool mirrored = false, ColorRemap remap = ColorRemap::kNone,
			  const Common::Array<ColorRemapRegion> *regions = nullptr);
	/**
	 * Recolor an already drawn region when enabled, preserving alpha and neutral pixels.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	void recolorRect(Common::Rect area, ColorRemap remap);

private:
	/** RGBA surface containing the composited output. */
	Graphics::Surface _surface;
	/** ScummVM-only setting for explicitly requested color remaps; absent from the original engine. */
	bool _colorBlindMode = false;
	/**
	 * Recolor a chromatic pixel while preserving its shading and neutral details.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	static void recolor(byte &r, byte &g, byte &b, ColorRemap remap);
	/** Disallow copying the output allocation. */
	Canvas(const Canvas &) = delete;
	/** Disallow assigning the output allocation. */
	Canvas &operator=(const Canvas &) = delete;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
