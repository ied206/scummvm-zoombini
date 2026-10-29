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

#include "trinket/ompp/canvas.h"

#include "common/util.h"

namespace Trinket {
namespace OM {

Canvas::Canvas(int width, int height) {
	_surface.create(width, height, Graphics::PixelFormat::createFormatRGBA32());
}

Canvas::~Canvas() {
	_surface.free();
}

void Canvas::recolor(byte &r, byte &g, byte &b, ColorRemap remap) {
	const int maximum = MAX<int>(r, MAX<int>(g, b));
	const int minimum = MIN<int>(r, MIN<int>(g, b));
	if (maximum - minimum < 24)
		return;
	if (remap == ColorRemap::kGreenToGray || remap == ColorRemap::kOrangeToWhite || remap == ColorRemap::kOrangeToDarkGray) {
		// Use every channel: orange shading can vary while its maximum stays fixed.
		int value = (77 * r + 150 * g + 29 * b + 128) / 256;
		if (remap == ColorRemap::kOrangeToWhite)
			value = (value + maximum) / 2;
		else if (remap == ColorRemap::kOrangeToDarkGray)
			value = (value * (255 + value) + 255) / 510;
		else
			value = 510 * value / (255 + value);
		r = g = b = value;
		return;
	}
	int red;
	int green;
	int blue;
	switch (remap) {
	case ColorRemap::kGreenToCyan:
		red = 0;
		green = 182;
		blue = 217;
		break;
	case ColorRemap::kPurpleOrMagentaToDarkPurple:
		red = 102;
		green = 0;
		blue = 68;
		break;
	default:
		return;
	}
	const int targetMaximum = MAX(red, MAX(green, blue));
	const int targetMinimum = MIN(red, MIN(green, blue));
	const int value = maximum * targetMaximum / 255;
	const int range = targetMaximum - targetMinimum;
	if (!range) {
		r = g = b = value;
		return;
	}
	const int sourceSaturation = (maximum - minimum) * 255 / maximum;
	const int targetSaturation = range * 255 / targetMaximum;
	const int chroma = value * (sourceSaturation + targetSaturation) / 510;
	const int base = value - chroma;
	r = base + chroma * (red - targetMinimum) / range;
	g = base + chroma * (green - targetMinimum) / range;
	b = base + chroma * (blue - targetMinimum) / range;
}

void Canvas::recolorRect(Common::Rect area, ColorRemap remap) {
	if (!_colorBlindMode || remap == ColorRemap::kNone)
		return;
	area.clip(Common::Rect(_surface.w, _surface.h));
	for (int y = area.top; y < area.bottom; y++) {
		for (int x = area.left; x < area.right; x++) {
			uint32 *pixel = static_cast<uint32 *>(_surface.getBasePtr(x, y));
			byte a;
			byte r;
			byte g;
			byte b;
			_surface.format.colorToARGB(*pixel, a, r, g, b);
			if (!a)
				continue;
			recolor(r, g, b, remap);
			*pixel = _surface.format.ARGBToColor(a, r, g, b);
		}
	}
}

void Canvas::blit(const Graphics::Surface &source, int x, int y, bool mirrored, ColorRemap remap,
				  const Common::Array<ColorRemapRegion> *regions) {
	// OM composites over destination RGB regardless of its stored alpha, rounds down
	// after division by 255 and writes opaque pixels. Graphics::ManagedSurface honors
	// destination alpha, while Graphics::alphaBlit divides by 256; neither preserves this result.
	assert(source.format == _surface.format);
	int left = MAX(0, -x);
	int top = MAX(0, -y);
	int right = MIN<int>(source.w, _surface.w - x);
	int bottom = MIN<int>(source.h, _surface.h - y);
	for (int row = top; row < bottom; row++) {
		for (int col = left; col < right; col++) {
			int sourceX = mirrored ? source.w - 1 - col : col;
			uint32 src = *static_cast<const uint32 *>(source.getBasePtr(sourceX, row));
			byte a;
			byte r;
			byte g;
			byte b;
			source.format.colorToARGB(src, a, r, g, b);
			if (!a)
				continue;
			if (_colorBlindMode) {
				ColorRemap pixelRemap = remap;
				if (regions) {
					for (const auto &region : *regions) {
						if (!region.bounds.contains(sourceX, row))
							continue;
						if (region.source == ColorSource::kRedDominant && (r <= g || r <= b))
							continue;
						if (region.source == ColorSource::kGreenDominant && (g <= r || g <= b))
							continue;
						pixelRemap = region.remap;
						break;
					}
				}
				if (pixelRemap != ColorRemap::kNone)
					recolor(r, g, b, pixelRemap);
			}
			uint32 *dest = static_cast<uint32 *>(_surface.getBasePtr(x + col, y + row));
			if (a != 255) {
				byte dr;
				byte dg;
				byte db;
				_surface.format.colorToRGB(*dest, dr, dg, db);
				r = (r * a + dr * (255 - a)) / 255;
				g = (g * a + dg * (255 - a)) / 255;
				b = (b * a + db * (255 - a)) / 255;
			}
			*dest = _surface.format.ARGBToColor(255, r, g, b);
		}
	}
}

} // namespace OM
} // namespace Trinket
