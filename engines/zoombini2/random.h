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

#include "common/platform.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Zoombini2 {

/** Persisted and runtime pseudo-random generator selection. */
enum class PrngAlgorithmKind : uint32 {
	/** Use the target platform's original linear congruential stream. */
	kOriginalPrng = 0,
	/** Use @ref Common::RandomSource. */
	kStandardPrng = 1,
};

/**
 * Provides the selectable random stream shared by game logic.
 *
 * The "prng_algorithm" option selects the target platform's 32-bit LCG or @ref Common::RandomSource.
 * The compatibility stream advances once per request and bounds its 15-bit output with a remainder operation.
 * Consequently, large requested ranges do not expand its output beyond 32767.
 * Changing the algorithm starts a new sequence when engine settings are applied.
 */
class Random {
private:
	/** Current 32-bit state of the target platform's original LCG. */
	uint32 _randState = 0;
	/** Detected game platform selecting the original recurrence and initial seed. */
	const Common::Platform _platform;
	/** ScummVM standard PRNG. */
	Common::RandomSource _scummRnd;
	/** Currently selected PRNG algorithm. */
	PrngAlgorithmKind _prngAlgorithm;
	/** Return one for Macintosh Original PRNG, or @p defaultSeed for the other selections. */
	uint32 getInitialSeed(uint32 defaultSeed) const;

	/**
	 * Advance the compatibility state and return an inclusive value from zero through @p max.
	 * The state is advanced even when @p max is zero.
	 */
	int32 getOriginalRandomNumber(int32 max);

public:
	/** Construct the shared stream for @p platform and select its algorithm from the target configuration. */
	Random(const Common::String &name, Common::Platform platform);

	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(PrngAlgorithmKind prngAlgorithm);
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
