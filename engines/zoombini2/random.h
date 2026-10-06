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

#ifndef ZOOMBINI2_RANDOM_H
#define ZOOMBINI2_RANDOM_H

#include "common/random.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "zoombini2/metaengine.h"

namespace Zoombini2 {

/**
 * Reimplement RNG of MSVC 6.0 CRT used by the original Windows releases.
 *
 * An engine variant for another original platform, such as Macintosh, must
 * identify that platform's runtime RNG before selecting this stream.
 *
 * The "prng_algorithm" configuration option selects this compatibility stream
 * or ScummVM's default @ref Common::RandomSource stream.
 * Changing the algorithm starts a new sequence when engine settings are applied.
 */
class Random {
private:
	/** State of the original PRNG (MSVC 6.0 CRT LCG) */
	uint32 _randState = 0;
	/** ScummVM standard PRNG. */
	Common::RandomSource _scummRnd;
	/** Currently selected PRNG algorithm. */
	Zoombini2MetaEngine::PrngAlgorithm _prngAlgorithm;

	/** 
	 * Generate one inclusive bounded value with the compatibility algorithm.
	 * Advance the compatibility state and return an iclusive value from zero through @p max.
	 * The state is advanced even when @p max is zero.
	 */
	int32 getOriginalRandomNumber(int32 max);

public:
	/** Construct the shared game stream and select its algorithm from the target configuration. */
	explicit Random(const Common::String &name);

	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(Zoombini2MetaEngine::PrngAlgorithm prngAlgorithm);
	/** Seed both backing streams. The compatibility stream preserves zero exactly. */
	void setSeed(uint32 seed);
	/** Return the current state of the selected stream. */
	uint32 getSeed() const;
	/** Generates new seed based on the current date/time */
	static uint32 generateNewSeed();

	/** Advance the stream and return an inclusive value in the range zero through @p max. */
	int32 getRandomNumber(int32 max);
	/**
	 * Advance the stream and return an inclusive signed value in the range @p min through @p max.
	 * Swap reversed endpoints and report a fatal error if their span exceeds INT_MAX.
	 */
	int32 getRandomNumberRng(int32 min, int32 max);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_RANDOM_H
