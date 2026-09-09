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

#include "common/debug.h"
#include "common/file.h"

#include "zoombini2/path.h"

namespace Zoombini2 {

// ============================================================================
// CurveSegment - cubic Bezier segment using 10-bit fixed-point math.
// ============================================================================

void CurveSegment::init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1,
						const Common::Point32 &end, int stepVal, int waitVal) {
	p0 = Common::Point32(start.x << 10, start.y << 10);
	cp0 = Common::Point32(control0.x << 10, control0.y << 10);
	cp1 = Common::Point32(control1.x << 10, control1.y << 10);
	p1 = Common::Point32(end.x << 10, end.y << 10);

	step = stepVal;
	waitInit = waitVal;
	waitRemain = waitVal;

	c2 = Common::Point32();
	c1 = Common::Point32();
	c0 = Common::Point32();
	paramT = 0;
	position = p0;
	startTime = 0;
}

/**
 * Compute cubic Bezier polynomial coefficients from four control points.
 *
 * Standard cubic Bezier: B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*CP0 + 3*(1-t)*t^2*CP1 + t^3*P1
 * Rearranged: B(t) = P0 + C0*t + C1*t^2 + C2*t^3
 * Where:
 *   C0 = 3*(CP0 - P0)
 *   C1 = 3*(CP1 - CP0) - C0
 *   C2 = (P1 - P0) - C0 - C1
 *
 * All values in <<10 fixed-point. Multiplies by 3072 (=3<<10) then >>10 = multiply by 3.
 */
void CurveSegment::computeCoeffs() {
	paramT = 0;

	// X coefficients
	int32 dxC0 = cp0.x - p0.x;
	int32 dxC1 = cp1.x - cp0.x;
	c0.x = static_cast<int32>((3072LL * dxC0) >> 10);
	c1.x = static_cast<int32>((3072LL * dxC1) >> 10) - c0.x;
	c2.x = (p1.x - p0.x) - c0.x - c1.x;

	// Y coefficients
	int32 dyC0 = cp0.y - p0.y;
	int32 dyC1 = cp1.y - cp0.y;
	c0.y = static_cast<int32>((3072LL * dyC0) >> 10);
	c1.y = static_cast<int32>((3072LL * dyC1) >> 10) - c0.y;
	c2.y = (p1.y - p0.y) - c0.y - c1.y;

	waitRemain = waitInit;
}

/**
 * Evaluate the Bezier curve at the current time.
 * paramT = step * ((tickCount - startTime) / 5)
 * When paramT > 950: segment near-complete, start wait countdown.
 * Returns false when segment fully complete (wait expired).
 */
bool CurveSegment::evaluate(uint32 tickCount, Common::Point32 &outPosition) {
	if (paramT <= 950) {
		// Advance parameter based on elapsed time
		uint32 elapsed = tickCount - startTime;
		int32 ticks = static_cast<int32>(elapsed / 5u);
		paramT = static_cast<int32>(((static_cast<int64>(step) * (static_cast<int64>(ticks) << 10)) >> 10));

		int32 t = paramT;
		int32 t2 = static_cast<int32>((static_cast<int64>(t) * t) >> 10);  // t^2
		int32 t3 = static_cast<int32>((static_cast<int64>(t2) * t) >> 10); // t^3

		// B(t) = P0 + C0*t + C1*t^2 + C2*t^3
		position.x = p0.x + static_cast<int32>((static_cast<int64>(c0.x) * t) >> 10) +
			static_cast<int32>((static_cast<int64>(c1.x) * t2) >> 10) + static_cast<int32>((static_cast<int64>(c2.x) * t3) >> 10);

		position.y = p0.y + static_cast<int32>((static_cast<int64>(c0.y) * t) >> 10) +
			static_cast<int32>((static_cast<int64>(c1.y) * t2) >> 10) + static_cast<int32>((static_cast<int64>(c2.y) * t3) >> 10);

		outPosition = Common::Point32(position.x >> 10, position.y >> 10);
		return true;
	}

	// paramT > 950: segment near-complete
	if (waitRemain > 0) {
		waitRemain -= 1;
		return true;
	}

	// The configured endpoint wait has expired.
	return false;
}

// ============================================================================
// PathObject - Bezier path composed of chained CurveSegments.
// ============================================================================

PathObject::PathObject()
	: currentSegment(0), looping(false), finished(false),
	  endPosition(), startTime(0) {
}

PathObject::~PathObject() {
	for (uint i = 0; i < segments.size(); i++)
		delete segments[i];
}

/**
 * Load a `.PAT` Bezier path file.
 *
 * Format:
 *   FIRST=coord:x0,y0,cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *   NEXT_N=coord:cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *
 * NEXT segments chain from the previous segment's endpoint.
 * Compatibility requires `NEXT` segments to reuse the `FIRST` line's step and wait values.
 */
PathObject *PathObject::loadFromPAT(const Common::Path &path) {
	Common::File f;
	if (!f.open(path)) {
		warning("PathObject: cannot open PAT '%s'", path.toString().c_str());
		return nullptr;
	}

	PathObject *obj = new PathObject();
	int firstStep = 1, firstWait = 0;

	// Read FIRST line
	Common::String line = f.readLine();
	if (line.hasPrefix("FIRST=coord:")) {
		int x0, y0, cx0, cy0, cx1, cy1, x1, y1, stepVal, waitVal;
		if (sscanf(line.c_str(),
				   "FIRST=coord:%d,%d,%d,%d,%d,%d,%d,%d step:%d wait:%d",
				   &x0, &y0, &cx0, &cy0, &cx1, &cy1, &x1, &y1,
				   &stepVal, &waitVal) >= 10) {
			CurveSegment *seg = new CurveSegment();
			seg->init(
				Common::Point32(x0, y0), Common::Point32(cx0, cy0), Common::Point32(cx1, cy1), Common::Point32(x1, y1), stepVal, waitVal);
			obj->segments.push_back(seg);
			firstStep = stepVal;
			firstWait = waitVal;
		}
	}

	// Read NEXT lines (start with 'N')
	while (!f.eos()) {
		line = f.readLine();
		if (line.empty() || line[0] != 'N')
			break;

		// Parse 6 coordinate values after skipping "NEXT_N=coord:"
		int ext, ncx0, ncy0, ncx1, ncy1, nx1, ny1;
		// Skip the leading 'N' and parse the remaining `EXT` record.
		if (sscanf(line.c_str() + 1,
				   "EXT_%d=coord:%d,%d,%d,%d,%d,%d",
				   &ext, &ncx0, &ncy0, &ncx1, &ncy1, &nx1, &ny1) >= 7) {
			CurveSegment *prev = obj->segments.back();
			CurveSegment *seg = new CurveSegment();
			// P0 comes from previous segment's P1
			seg->init(
				Common::Point32(prev->p1.x >> 10, prev->p1.y >> 10), Common::Point32(ncx0, ncy0),
				Common::Point32(ncx1, ncy1), Common::Point32(nx1, ny1), firstStep, firstWait);
			obj->segments.push_back(seg);
		}
	}

	if (obj->segments.empty()) {
		delete obj;
		return nullptr;
	}

	// Set endpoint from last segment
	CurveSegment *last = obj->segments.back();
	obj->endPosition = Common::Point32(last->p1.x >> 10, last->p1.y >> 10);

	return obj;
}

/**
 * Start path evaluation. Sets startTime on all segments and computes
 * coefficients for the first segment.
 */
void PathObject::start(uint32 tickCount) {
	currentSegment = 0;
	finished = false;
	startTime = tickCount;

	for (uint i = 0; i < segments.size(); i++)
		segments[i]->startTime = tickCount;

	if (!segments.empty())
		segments[0]->computeCoeffs();
}

/**
 * Advance the path evaluation. Returns true if still walking,
 * false if the entire path is complete.
 * @p outPosition receives the current screen position.
 */
bool PathObject::advance(uint32 tickCount, Common::Point32 &outPosition) {
	if (finished) {
		outPosition = endPosition;
		return false;
	}

	if (currentSegment >= static_cast<int>(segments.size())) {
		finished = true;
		outPosition = endPosition;
		return false;
	}

	CurveSegment *seg = segments[currentSegment];
	if (seg->evaluate(tickCount, outPosition))
		return true;

	// Advance after the current segment completes.
	currentSegment += 1;
	if (currentSegment >= static_cast<int>(segments.size())) {
		// All segments done
		finished = true;
		outPosition = endPosition;
		return false;
	}

	// Compute coefficients for the next segment and evaluate it
	CurveSegment *next = segments[currentSegment];
	next->computeCoeffs();
	next->startTime = tickCount;
	next->evaluate(tickCount, outPosition);
	return true;
}

} // End of namespace Zoombini2
