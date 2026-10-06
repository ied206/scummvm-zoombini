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

#include <limits.h>

#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "gui/EventRecorder.h"

#include "zoombini2/metaengine.h"
#include "zoombini2/random.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

Random::Random(const Common::String &name) : _scummRnd(name) {
	const int prngAlgorithmVal = ConfMan.getInt(Zoombini2MetaEngine::kConfigPrngAlgorithm);
	_prngAlgorithm = static_cast<Zoombini2MetaEngine::PrngAlgorithm>(prngAlgorithmVal);

#ifdef ENABLE_EVENTRECORDER
	setSeed(g_eventRec.getRandomSeed(name));
#else
	setSeed(generateNewSeed());
#endif
}

void Random::setAlgorithm(Zoombini2MetaEngine::PrngAlgorithm prngAlgorithm) {
	if (_prngAlgorithm == prngAlgorithm)
		return;

	_prngAlgorithm = prngAlgorithm;
	setSeed(generateNewSeed());
}

void Random::setSeed(uint32 seed) {
	_randState = seed;
	_scummRnd.setSeed(seed);
}

uint32 Random::getSeed() const {
	switch (_prngAlgorithm) {
	case Zoombini2MetaEngine::PrngAlgorithm::kOriginalPrng:
		return _randState;
	case Zoombini2MetaEngine::PrngAlgorithm::kStandardPrng:
	default:
		return _scummRnd.getSeed();
	}
}

uint32 Random::generateNewSeed() {
	return Common::RandomSource::generateNewSeed();
}

int32 Random::getOriginalRandomNumber(int32 max) {
	assert(0 <= max);

	// MSVC 6.0 CRT rand() advances this unsigned 32-bit LCG and returns bits 16 through 30.
	// The state is advanced even though the max limit is 0.
	_randState = 214013u * _randState + 2531011u;
	const int32 result = static_cast<int32>((_randState >> 16) & 0x7FFFu);
	if (max == INT_MAX)
		return result;
	return result % (max + 1);
}

int32 Random::getRandomNumber(int32 max) {
	assert(0 <= max);

	switch (_prngAlgorithm) {
	case Zoombini2MetaEngine::PrngAlgorithm::kStandardPrng:
		return static_cast<int32>(_scummRnd.getRandomNumber(static_cast<uint32>(max)));
	case Zoombini2MetaEngine::PrngAlgorithm::kOriginalPrng:
	default:
		return getOriginalRandomNumber(max);
	}
}

int32 Random::getRandomNumberRng(int32 min, int32 max) {
	if (max < min) {
		warning("Zoombini2Random::getRandomNumberRng: max(%d) is smaller than min(%d), swapping", max, min);
		SWAP<int32>(min, max);
	}

	// Cast each endpoint before subtraction to avoid signed overflow.
	// Unsigned subtraction wraps modulo 2^32 and yields the exact span for sorted endpoints, even when the range crosses zero.
	// For example, min = -10 and max = 10 give a span of 20.
	const uint32 span = static_cast<uint32>(max) - static_cast<uint32>(min);
	if (static_cast<uint32>(INT_MAX) < span)
		error("Random::getRandomNumberRng: range exceeds INT_MAX");

	// With an offset in [0, span], this signed sum stays in [min, max].
	return min + getRandomNumber(static_cast<int32>(span));
}

} // End of namespace Zoombini2
