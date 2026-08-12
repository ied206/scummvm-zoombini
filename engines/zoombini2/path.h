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

#ifndef ZOOMBINI2_PATH_H
#define ZOOMBINI2_PATH_H

#include "common/array.h"
#include "common/path.h"

namespace Zoombini2 {

/**
 * CurveSegment — single cubic Bezier segment (10-bit fixed-point math).
 * Original: CurveSegment__Init_405EA0, 96 bytes (0x60).
 *
 * Points P0, CP0, CP1, P1 define a standard cubic Bezier curve.
 * All coordinates stored as screen_value << 10.
 * Polynomial form: B(t) = P0 + C0*t + C1*t^2 + C2*t^3
 * where t in [0, 1024] represents [0.0, 1.0].
 */
struct CurveSegment {
	// Control points (<<10 fixed-point)
	int32 p0x, p0y;     // Start point
	int32 cp0x, cp0y;   // Control point 0
	int32 cp1x, cp1y;   // Control point 1
	int32 p1x, p1y;     // End point

	// Polynomial coefficients (cubic, quadratic, linear)
	int32 c2x, c1x, c0x;
	int32 c2y, c1y, c0y;

	// Evaluation state
	int32 paramT;        // Current parameter (0..1024+)
	int32 posX;          // Current position X (<<10)
	int32 posY;          // Current position Y (<<10)

	// Speed/timing
	int32 step;          // Speed multiplier
	int32 waitInit;      // Initial wait value
	int32 waitRemain;    // Remaining wait countdown
	uint32 startTime;    // Tick when evaluation started

	void init(int x0, int y0, int cx0, int cy0,
	          int cx1, int cy1, int x1, int y1,
	          int stepVal, int waitVal);
	void computeCoeffs();
	bool evaluate(uint32 tickCount, int &outX, int &outY);
};

/**
 * PathObject — bezier path composed of CurveSegments.
 * Original: PathObject__LoadFromPAT_45BA80, 284 bytes (0x11C).
 * Supports up to 64 chained segments.
 */
struct PathObject {
	Common::Array<CurveSegment *> segments;
	int currentSegment;
	bool looping;
	bool finished;
	int32 endX;          // Final endpoint X (screen coords)
	int32 endY;          // Final endpoint Y (screen coords)
	uint32 startTime;

	PathObject();
	~PathObject();

	static PathObject *loadFromPAT(const Common::Path &path);
	void start(uint32 tickCount);
	bool advance(uint32 tickCount, int &outX, int &outY);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PATH_H
