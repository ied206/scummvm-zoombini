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

#ifndef ZOOMBINI2_PATH_H
#define ZOOMBINI2_PATH_H

#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"

namespace Zoombini2 {

/**
 * Runtime evaluator for one cubic Bezier segment.
 *
 * Control points, polynomial coefficients, and evaluated positions use
 * 10-bit fixed-point coordinates. @ref CurveSegment::evaluate advances the
 * segment from the stored start tick, step, and optional initial wait.
 */
struct CurveSegment {
	/** Fixed-point start coordinate. */
	Common::Point32 p0;
	/** Fixed-point coordinate of the first control point. */
	Common::Point32 cp0;
	/** Fixed-point coordinate of the second control point. */
	Common::Point32 cp1;
	/** Fixed-point end coordinate. */
	Common::Point32 p1;

	/** Cubic coefficient pair. */
	Common::Point32 c2;
	/** Quadratic coefficient pair. */
	Common::Point32 c1;
	/** Linear coefficient pair. */
	Common::Point32 c0;

	/** Current parameter in the fixed-point interval beginning at zero. */
	int32 paramT;
	/** Current fixed-point position. */
	Common::Point32 position;

	/** Parameter advancement applied by each timing step. */
	int32 step;
	/** Initial wait value loaded for this segment. */
	int32 waitInit;
	/** Remaining wait intervals before movement begins. */
	int32 waitRemain;
	/** Gameplay tick at which this segment started. */
	uint32 startTime;

	/** Initialize control points, timing values, and polynomial coefficients. */
	void init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1,
			  const Common::Point32 &end, int stepVal, int waitVal);
	/** Derive polynomial coefficients from the four control points. */
	void computeCoeffs();
	/** Evaluate the position at @p tickCount and report whether the segment remains active. */
	bool evaluate(uint32 tickCount, Common::Point32 &outPosition);
};

/**
 * Owns and advances the chained segments loaded from one PAT path.
 *
 * Non-looping paths stop at their final endpoint. Looping paths return to the
 * first segment after the last segment completes.
 */
struct PathObject {
	/** Ordered segments owned by this path. */
	Common::Array<CurveSegment *> segments;
	/** Index of the segment currently being evaluated. */
	int currentSegment;
	/** Whether the path restarts after the last segment. */
	bool looping;
	/** Whether a non-looping path has reached its final endpoint. */
	bool finished;
	/** Final endpoint in screen pixels. */
	Common::Point32 endPosition;
	/** Gameplay tick at which the path started. */
	uint32 startTime;

	/** Construct an empty, inactive path. */
	PathObject();
	/** Release every owned segment. */
	~PathObject();

	/** Parse and return a newly allocated path, or nullptr on failure. */
	static PathObject *loadFromPAT(const Common::Path &path);
	/** Reset segment state and begin evaluation at @p tickCount. */
	void start(uint32 tickCount);
	/** Advance to @p tickCount and write the current screen position. */
	bool advance(uint32 tickCount, Common::Point32 &outPosition);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PATH_H
