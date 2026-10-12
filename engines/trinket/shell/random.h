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

/** Shared gameplay random state, including script and native consumers. */
class Random {
public:
	/** Persisted and runtime pseudo-random generator selection. */
	enum class PrngAlgorithm : uint32 {
		kOriginalPrng = 0,
		kStandardPrng = 1,
	};
	static constexpr const char *kPrngAlgorithmOption = "prng_algorithm";

	/** Read the selected algorithm from the target configuration. */
	static PrngAlgorithm getPrngAlgorithm(const Common::String &domain = Common::String());
	Random();

	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(PrngAlgorithm prngAlgorithm);
	void setSeed(uint32 seed);

	/** CRT-compatible raw draw; advances even when a caller has a singleton range. */
	uint32 draw();

	/** Shell RandomNumber: inclusive endpoints; equal endpoints consume no draw. */
	int number(int minimum, int maximum);

	/** Shell exclusion advances a matching result by one, without redrawing. */
	int excluding(int count, int excluded = -1);

private:
	Common::RandomSource _fallback{"trinket"};
	uint32 _state = _fallback.getSeed();

	/** Currently selected PRNG algorithm. */
	PrngAlgorithm _prngAlgorithm;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
