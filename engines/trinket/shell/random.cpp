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

#include "trinket/shell/random.h"

#include "common/config-manager.h"
#include "common/util.h"

namespace Trinket {
namespace Shell {

constexpr const char *Random::kPrngAlgorithmOption;

Random::PrngAlgorithm Random::getPrngAlgorithm(const Common::String &domain) {
	const int prngAlgorithmVal = ConfMan.getInt(kPrngAlgorithmOption, domain);
	const PrngAlgorithm prngAlgorithm = static_cast<PrngAlgorithm>(prngAlgorithmVal);
	if (prngAlgorithm == PrngAlgorithm::kStandardPrng)
		return prngAlgorithm;
	return PrngAlgorithm::kOriginalPrng;
}

Random::Random() : _prngAlgorithm(getPrngAlgorithm()) {
}

void Random::setAlgorithm(PrngAlgorithm prngAlgorithm) {
	if (_prngAlgorithm == prngAlgorithm)
		return;

	_prngAlgorithm = prngAlgorithm;
	setSeed(Common::RandomSource::generateNewSeed());
}

void Random::setSeed(uint32 seed) {
	_state = seed;
	_fallback.setSeed(seed);
}

uint32 Random::draw() {
	if (_prngAlgorithm == PrngAlgorithm::kStandardPrng)
		return _fallback.getRandomNumber(32767);
	// The Windows MSVCRT generator wraps its state at 32 bits and returns 15 bits.
	_state = _state * 214013u + 2531011u;
	return (_state >> 16) & 32767u;
}

int Random::excluding(int count, int excluded) {
	if (count <= 0)
		return 0;
	int result = draw() % count;
	if (result == excluded)
		result = (result + 1) % count;
	return result;
}

int Random::number(int minimum, int maximum) {
	if (minimum == maximum)
		return minimum;
	if (maximum < minimum)
		SWAP(minimum, maximum);
	assert(static_cast<int64>(maximum) - minimum < 2147483647);
	return minimum + excluding(maximum - minimum + 1);
}

} // namespace Shell
} // namespace Trinket
