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

#ifndef ZOOMBINI2_RANDOM_H
#define ZOOMBINI2_RANDOM_H

#include "common/random.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Zoombini2 {

/**
 * Reimplement RNG of MSVC 6.0 CRT used by the original Windows releases.
 *
 * An engine variant for another original platform, such as Macintosh, must
 * identify that platform's runtime RNG before selecting this stream.
 *
 * The "original_prng" configuration option selects this compatibility stream
 * or ScummVM's default @ref Common::RandomSource stream at construction time.
 */
class Random {
private:
	/** Complete unsigned 32-bit MSVC 6.0 CRT linear-congruential state. */
	uint32 _randState = 0;
	/** ScummVM random-number stream used when compatibility mode is disabled. */
	Common::RandomSource _scummRnd;
	/** Whether calls use the original Windows compatibility algorithm. */
	bool _useOriginal;

	/** Advance the compatibility state and return an inclusive value from zero through @p max, including when @p max is zero. */
	int32 getOriginalRandomNumber(int32 max);

public:
	/** Construct the shared game stream and select its algorithm from "original_prng". */
	explicit Random(const Common::String &name);

	/** Seed both backing streams. The compatibility stream preserves zero exactly. */
	void setSeed(uint32 seed);
	/** Return the current state of the selected stream. */
	uint32 getSeed() const { return _useOriginal ? _randState : _scummRnd.getSeed(); }
	/** Generates new seed based on the current date/time */
	static uint32 generateNewSeed();

	/** Advance the stream and return an inclusive value in the range zero through @p max. */
	int32 getRandomNumber(int32 max);
	/** Advance the stream and return an inclusive signed value in the range @p min through @p max. */
	int32 getRandomNumberRng(int32 min, int32 max);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_RANDOM_H
