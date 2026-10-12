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

void Canvas::blit(const Graphics::Surface &source, int x, int y, bool mirrored) {
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
			byte a, r, g, b;
			source.format.colorToARGB(src, a, r, g, b);
			if (!a)
				continue;
			uint32 *dest = static_cast<uint32 *>(_surface.getBasePtr(x + col, y + row));
			if (a != 255) {
				byte dr, dg, db;
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
