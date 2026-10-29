/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_FRAME_CLOCK_H
#define TRINKET_FRAME_CLOCK_H

#include "common/scummsys.h"

namespace Trinket {

/** Accumulate active milliseconds from one timestamp per frame or input event. */
class FrameClock {
public:
	/** Start a fresh timeline at zero using the supplied source timestamp. */
	void reset(uint32 time) {
		_time = 0;
		_lastTime = time;
		_delta = 0;
		_started = true;
	}
	/** Sample source time, excluding this interval when the owner is stopped or paused. */
	void update(uint32 time, bool running = true) {
		if (!_started) {
			reset(time);
			return;
		}
		_delta = 0;
		if (running)
			_delta = time - _lastTime;
		_lastTime = time;
		_time += _delta;
	}
	/** Return active milliseconds since reset, with unsigned clock wrapping. */
	uint32 now() const { return _time; }
	/** Return active milliseconds contributed by the latest sample. */
	uint32 delta() const { return _delta; }
	/** Return active milliseconds since an earlier value of @ref FrameClock::now. */
	uint32 elapsed(uint32 start) const { return _time - start; }
	/** Test a deadline less than half a clock cycle away, including across unsigned wrap. */
	static bool reached(uint32 time, uint32 deadline) { return 0 <= static_cast<int32>(time - deadline); }
	/** Convert milliseconds to completed frames without overflowing the intermediate product. */
	static uint32 frames(uint32 time, uint fps) { return static_cast<uint32>(static_cast<uint64>(time) * fps / 1000); }
	/** Convert a frame count to whole milliseconds at a nonzero frame rate. */
	static uint32 milliseconds(uint32 frames, uint fps) {
		assert(fps);
		return static_cast<uint32>(static_cast<uint64>(frames) * 1000 / fps);
	}

private:
	/** Accumulated active time in milliseconds. */
	uint32 _time = 0;
	/** Source timestamp of the previous sample in milliseconds. */
	uint32 _lastTime = 0;
	/** Active duration of the latest sample in milliseconds. */
	uint32 _delta = 0;
	/** Whether a source timestamp has been sampled, allowing zero as a valid timestamp. */
	bool _started = false;
};

} // End of namespace Trinket

#endif
