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

#include "graphics/surface.h"

namespace Trinket {
namespace OM {

class Canvas {
public:
	Canvas(int width, int height);
	~Canvas();
	Graphics::Surface &surface() { return _surface; }
	const Graphics::Surface &surface() const { return _surface; }
	void blit(const Graphics::Surface &source, int x, int y, bool mirrored = false);

private:
	Graphics::Surface _surface;
	Canvas(const Canvas &) = delete;
	Canvas &operator=(const Canvas &) = delete;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
