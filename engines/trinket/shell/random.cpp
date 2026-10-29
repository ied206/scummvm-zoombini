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
#include "common/textconsole.h"
#include "common/util.h"
#include "trinket/metaengine.h"

#ifdef ENABLE_EVENTRECORDER
#include "gui/EventRecorder.h"
#endif

namespace Trinket {
namespace Shell {

Random::Random(const Common::String &name) : _scummRnd(name) {
	const int prngAlgorithmVal = ConfMan.getInt(TrinketMetaEngine::kOptionPrngAlgorithm);
	_prngAlgorithm = static_cast<PrngAlgorithmKind>(prngAlgorithmVal);

#ifdef ENABLE_EVENTRECORDER
	setSeed(g_eventRec.getRandomSeed(name));
#else
	setSeed(generateNewSeed());
#endif
}

void Random::setAlgorithm(PrngAlgorithmKind prngAlgorithm) {
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
	case PrngAlgorithmKind::kStandardPrng:
		return _scummRnd.getSeed();
	case PrngAlgorithmKind::kOriginalPrng:
	default:
		return _randState;
	}
}

uint32 Random::generateNewSeed() {
	return Common::RandomSource::generateNewSeed();
}

int32 Random::getOriginalRandomNumber(int32 max) {
	assert(0 <= max);
	// MSVC 6.0 CRT rand() advances this unsigned 32-bit LCG and returns bits 16 through 30.
	_randState = 214013u * _randState + 2531011u;
	const int32 result = static_cast<int32>((_randState >> 16) & 0x7FFFu);
	if (max == INT32_MAX)
		return result;
	return result % (max + 1);
}

int32 Random::getRandomNumber(int32 max) {
	assert(0 <= max);
	switch (_prngAlgorithm) {
	case PrngAlgorithmKind::kStandardPrng:
		return static_cast<int32>(_scummRnd.getRandomNumber(static_cast<uint32>(max)));
	case PrngAlgorithmKind::kOriginalPrng:
	default:
		return getOriginalRandomNumber(max);
	}
}

int32 Random::getRandomNumberRng(int32 min, int32 max) {
	if (min == max)
		return min;
	if (max < min)
		SWAP(min, max);
	// Unsigned endpoint subtraction preserves cross-zero spans without signed overflow.
	const uint32 span = static_cast<uint32>(max) - static_cast<uint32>(min);
	if (INT32_MAX < span)
		error("Random::getRandomNumberRng: range exceeds INT32_MAX");
	return min + getRandomNumber(static_cast<int32>(span));
}

int32 Random::getRandomNumberExcluding(int32 count, int32 excluded) {
	if (count <= 0)
		return 0;
	int32 result = getRandomNumber(count - 1);
	if (result == excluded)
		result = (result + 1) % count;
	return result;
}
} // namespace Shell
} // namespace Trinket
