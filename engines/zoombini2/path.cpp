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
// CurveSegment — cubic Bezier segment (10-bit fixed-point math).
// Original: CurveSegment__Init_405EA0,
//           CurveSegment__ComputeCoeffs_405F10,
//           CurveSegment__Evaluate_406000.
// ============================================================================

void CurveSegment::init(int x0, int y0, int cx0, int cy0,
                        int cx1, int cy1, int x1, int y1,
                        int stepVal, int waitVal) {
	p0x = x0 << 10;
	p0y = y0 << 10;
	cp0x = cx0 << 10;
	cp0y = cy0 << 10;
	cp1x = cx1 << 10;
	cp1y = cy1 << 10;
	p1x = x1 << 10;
	p1y = y1 << 10;

	step = stepVal;
	waitInit = waitVal;
	waitRemain = waitVal;

	c2x = c1x = c0x = 0;
	c2y = c1y = c0y = 0;
	paramT = 0;
	posX = p0x;
	posY = p0y;
	startTime = 0;
}

/**
 * Compute cubic Bezier polynomial coefficients from 4 control points.
 * Original: CurveSegment__ComputeCoeffs_405F10.
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
	int32 dxC0 = cp0x - p0x;
	int32 dxC1 = cp1x - cp0x;
	c0x = (int32)((3072LL * dxC0) >> 10);
	c1x = (int32)((3072LL * dxC1) >> 10) - c0x;
	c2x = (p1x - p0x) - c0x - c1x;

	// Y coefficients
	int32 dyC0 = cp0y - p0y;
	int32 dyC1 = cp1y - cp0y;
	c0y = (int32)((3072LL * dyC0) >> 10);
	c1y = (int32)((3072LL * dyC1) >> 10) - c0y;
	c2y = (p1y - p0y) - c0y - c1y;

	waitRemain = waitInit;
}

/**
 * Evaluate the Bezier curve at the current time.
 * Original: CurveSegment__Evaluate_406000.
 *
 * paramT = step * ((tickCount - startTime) / 5)
 * When paramT > 950: segment near-complete, start wait countdown.
 * Returns false when segment fully complete (wait expired).
 */
bool CurveSegment::evaluate(uint32 tickCount, int &outX, int &outY) {
	if (paramT <= 950) {
		// Advance parameter based on elapsed time
		uint32 elapsed = tickCount - startTime;
		int32 ticks = (int32)(elapsed / 5u);
		paramT = (int32)(((int64)step * ((int64)ticks << 10)) >> 10);

		int32 t = paramT;
		int32 t2 = (int32)(((int64)t * t) >> 10);        // t^2
		int32 t3 = (int32)(((int64)t2 * t) >> 10);       // t^3

		// B(t) = P0 + C0*t + C1*t^2 + C2*t^3
		posX = p0x
			+ (int32)(((int64)c0x * t) >> 10)
			+ (int32)(((int64)c1x * t2) >> 10)
			+ (int32)(((int64)c2x * t3) >> 10);

		posY = p0y
			+ (int32)(((int64)c0y * t) >> 10)
			+ (int32)(((int64)c1y * t2) >> 10)
			+ (int32)(((int64)c2y * t3) >> 10);

		outX = posX >> 10;
		outY = posY >> 10;
		return true;
	}

	// paramT > 950: segment near-complete
	if (waitRemain > 0) {
		waitRemain--;
		return true;
	}

	// Wait expired — segment fully complete
	return false;
}

// ============================================================================
// PathObject — bezier path composed of chained CurveSegments.
// Original: PathObject__LoadFromPAT_45BA80.
// ============================================================================

PathObject::PathObject()
	: currentSegment(0), looping(false), finished(false),
	  endX(0), endY(0), startTime(0) {
}

PathObject::~PathObject() {
	for (uint i = 0; i < segments.size(); i++)
		delete segments[i];
}

/**
 * Load a .PAT bezier path file.
 * Original: PathObject__LoadFromPAT_45BA80.
 *
 * Format:
 *   FIRST=coord:x0,y0,cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *   NEXT_N=coord:cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *
 * NEXT segments chain from the previous segment's endpoint.
 * The step/wait values in NEXT lines are not parsed by the original
 * (sscanf format mismatch); values from the FIRST line are reused.
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
			seg->init(x0, y0, cx0, cy0, cx1, cy1, x1, y1, stepVal, waitVal);
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
		// Skip the 'N' and parse as "EXT_%d=coord:..." (matching original behavior)
		if (sscanf(line.c_str() + 1,
		           "EXT_%d=coord:%d,%d,%d,%d,%d,%d",
		           &ext, &ncx0, &ncy0, &ncx1, &ncy1, &nx1, &ny1) >= 7) {
			CurveSegment *prev = obj->segments.back();
			CurveSegment *seg = new CurveSegment();
			// P0 comes from previous segment's P1
			seg->init(prev->p1x >> 10, prev->p1y >> 10,
			          ncx0, ncy0, ncx1, ncy1, nx1, ny1,
			          firstStep, firstWait);
			obj->segments.push_back(seg);
		}
	}

	if (obj->segments.empty()) {
		delete obj;
		return nullptr;
	}

	// Set endpoint from last segment
	CurveSegment *last = obj->segments.back();
	obj->endX = last->p1x >> 10;
	obj->endY = last->p1y >> 10;

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
 * outX/outY receive the current screen position.
 */
bool PathObject::advance(uint32 tickCount, int &outX, int &outY) {
	if (finished) {
		outX = endX;
		outY = endY;
		return false;
	}

	if (currentSegment >= (int)segments.size()) {
		finished = true;
		outX = endX;
		outY = endY;
		return false;
	}

	CurveSegment *seg = segments[currentSegment];
	if (seg->evaluate(tickCount, outX, outY))
		return true;

	// Current segment complete — advance to next
	currentSegment++;
	if (currentSegment >= (int)segments.size()) {
		// All segments done
		finished = true;
		outX = endX;
		outY = endY;
		return false;
	}

	// Compute coefficients for the next segment and evaluate it
	CurveSegment *next = segments[currentSegment];
	next->computeCoeffs();
	next->startTime = tickCount;
	next->evaluate(tickCount, outX, outY);
	return true;
}

} // End of namespace Zoombini2
