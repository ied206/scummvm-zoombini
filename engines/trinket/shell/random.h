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

#ifndef TRINKET_SHELL_RANDOM_H
#define TRINKET_SHELL_RANDOM_H

#include "common/random.h"
#include "common/str.h"

namespace Trinket {
namespace Shell {

/** Persisted and runtime pseudo-random generator selection. */
enum class PrngAlgorithmKind : uint32 {
	/** Use the MSVC-compatible linear congruential stream. */
	kOriginalPrng = 0,
	/** Use @ref Common::RandomSource. */
	kStandardPrng = 1,
};

/**
 * Provides the selectable random stream shared by scripts and native game objects.
 * The compatibility stream advances a 32-bit LCG and maps its 15-bit output with a remainder operation.
 * The standard stream uses @ref Common::RandomSource with the requested upper bound.
 * Changing the algorithm starts a new sequence when engine settings are applied.
 */
class Random {
private:
	/** Current 32-bit state of the MSVC-compatible LCG. */
	uint32 _randState = 0;
	/** ScummVM standard PRNG registered under the constructor's stream name. */
	Common::RandomSource _scummRnd;
	/** Currently selected PRNG algorithm. */
	PrngAlgorithmKind _prngAlgorithm;

	/** Advance the compatibility stream once and return an inclusive value from zero through @p max. */
	int32 getOriginalRandomNumber(int32 max);

public:
	/** Construct the named gameplay stream and select its configured algorithm and recorded or generated seed. */
	explicit Random(const Common::String &name);
	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(PrngAlgorithmKind prngAlgorithm);
	/** Seed both streams, preserving zero in the compatibility stream while @ref Common::RandomSource substitutes a nonzero seed. */
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
	 * Return an inclusive signed value between @p min and @p max, swapping reversed endpoints.
	 * Shell RandomNumber returns equal endpoints without consuming a draw.
	 * Report a fatal error if the endpoint span exceeds INT32_MAX.
	 */
	int32 getRandomNumberRng(int32 min, int32 max);
	/**
	 * Draw an index below @p count and advance a matching @p excluded index by one modulo @p count, without redrawing.
	 * Nonpositive counts return zero without consuming a draw; a count of one still consumes a draw.
	 */
	int32 getRandomNumberExcluding(int32 count, int32 excluded = -1);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
