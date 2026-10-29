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

#ifndef TRINKET_BEZIER_H
#define TRINKET_BEZIER_H

#include "common/rect.h"

namespace Trinket {

/** A cubic Bezier segment with control points in pixel coordinates and floating-point evaluation. */
class CubicBezierCurve {
public:
	/** Construct an uninitialized curve. */
	CubicBezierCurve() = default;
	/** Assign the four control points in pixels and precompute polynomial coefficients. */
	void init(const Common::Point32 &start, const Common::Point32 &ctrl0, const Common::Point32 &ctrl1, const Common::Point32 &end) {
		const FloatPoint startPoint(static_cast<float>(start.x), static_cast<float>(start.y));
		const FloatPoint ctrl0Point(static_cast<float>(ctrl0.x), static_cast<float>(ctrl0.y));
		const FloatPoint ctrl1Point(static_cast<float>(ctrl1.x), static_cast<float>(ctrl1.y));
		const FloatPoint endPoint(static_cast<float>(end.x), static_cast<float>(end.y));
		const float three = 3.0f;
		_start = startPoint;
		_linear = (ctrl0Point - startPoint) * three;
		_quadratic = (ctrl1Point - ctrl0Point) * three - _linear;
		_cubic = endPoint - startPoint - _linear - _quadratic;
	}
	/** Evaluate the supplied dimensionless parameter without clamping and return subpixel coordinates in pixels. */
	void evaluate(float parameter, float &x, float &y) const {
		const float squaredParameter = parameter * parameter;
		const float cubedParameter = squaredParameter * parameter;
		const FloatPoint position = _start + _linear * parameter + _quadratic * squaredParameter + _cubic * cubedParameter;
		x = position.x;
		y = position.y;
	}

private:
	/** A floating-point pixel coordinate used by the curve evaluator. */
	struct FloatPoint {
		/** Horizontal pixel coordinate. */
		float x = 0.0f;
		/** Vertical pixel coordinate. */
		float y = 0.0f;

		/** Construct a point at the origin. */
		FloatPoint() = default;
		/** Construct a point from floating-point pixel coordinates. */
		FloatPoint(float xValue, float yValue) : x(xValue), y(yValue) {}
		/** Add two points component-wise. */
		FloatPoint operator+(const FloatPoint &right) const { return FloatPoint(x + right.x, y + right.y); }
		/** Subtract two points component-wise. */
		FloatPoint operator-(const FloatPoint &right) const { return FloatPoint(x - right.x, y - right.y); }
		/** Scale both coordinates by a floating-point value. */
		FloatPoint operator*(float multiplier) const { return FloatPoint(x * multiplier, y * multiplier); }
	};

	/** Start point of the cubic segment in floating-point pixels. */
	FloatPoint _start;
	/** Linear coefficient of the cubic polynomial in floating-point pixels. */
	FloatPoint _linear;
	/** Quadratic coefficient of the cubic polynomial in floating-point pixels. */
	FloatPoint _quadratic;
	/** Cubic coefficient of the cubic polynomial in floating-point pixels. */
	FloatPoint _cubic;
};

} // End of namespace Trinket

#endif // TRINKET_BEZIER_H
