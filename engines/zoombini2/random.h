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
 * Provides the selectable random stream shared by game logic.
 *
 * The "prng_algorithm" option selects the MSVC-compatible 32-bit LCG or @ref Common::RandomSource.
 * The compatibility stream advances once per request and bounds its 15-bit output with a remainder operation.
 * Consequently, large requested ranges do not expand its output beyond 32767.
 * Changing the algorithm starts a new sequence when engine settings are applied.
 */
class Random {
private:
	/** Current 32-bit state of the MSVC-compatible LCG. */
	uint32 _randState = 0;
	/** ScummVM standard PRNG. */
	Common::RandomSource _scummRnd;
	/** Currently selected PRNG algorithm. */
	Zoombini2MetaEngine::PrngAlgorithm _prngAlgorithm;

	/**
	 * Advance the compatibility state and return an inclusive value from zero through @p max.
	 * The state is advanced even when @p max is zero.
	 */
	int32 getOriginalRandomNumber(int32 max);

public:
	/** Construct the shared game stream and select its algorithm from the target configuration. */
	explicit Random(const Common::String &name);

	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(Zoombini2MetaEngine::PrngAlgorithm prngAlgorithm);
	/**
	 * Seed both backing streams.
	 * The compatibility stream preserves zero exactly; @ref Common::RandomSource substitutes a nonzero seed.
	 */
	void setSeed(uint32 seed);
	/** Return the current state of the selected stream. */
	uint32 getSeed() const;
	/** Obtain a seed from @ref Common::RandomSource::generateNewSeed, honoring ScummVM's configured random seed. */
	static uint32 generateNewSeed();

	/**
	 * Advance the selected stream once and return an inclusive value from zero through @p max.
	 * @param max Nonnegative upper bound; zero still consumes a draw.
	 */
	int32 getRandomNumber(int32 max);
	/**
	 * Advance the stream and return an inclusive signed value in the range @p min through @p max.
	 * Swap reversed endpoints and report a fatal error if their span exceeds INT_MAX.
	 */
	int32 getRandomNumberRng(int32 min, int32 max);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_RANDOM_H
