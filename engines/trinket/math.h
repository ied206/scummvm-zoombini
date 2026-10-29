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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRINKET_MATH_H
#define TRINKET_MATH_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace Trinket {

/** Calculate distances used by movement and animation. */
class Math {
public:
	/**
	 * Return the pixel distance from horizontal and vertical deltas.
	 * @tparam T Floating-point type used for the squared length, square root and result.
	 */
	template<class T = double>
	static T distance(int32 dx, int32 dy) {
		const double x = dx;
		const double y = dy;
		return sqrt(static_cast<T>(x * x + y * y));
	}
	/**
	 * Return the pixel length of the supplied displacement.
	 * @tparam T Floating-point type used for the square root and result.
	 */
	template<class T = double>
	static T distance(const Common::Point &delta) {
		return distance<T>(delta.x, delta.y);
	}
};

} // End of namespace Trinket

#endif // TRINKET_MATH_H
