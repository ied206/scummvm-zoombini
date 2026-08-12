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

#include "common/random.h"
#include "common/str.h"
#include "zoombini2/zoombini.h"

namespace Zoombini2 {

Zoombini::Zoombini() {
	_status = 0;
	_featureByte0 = 0;
	_featureA = 0;
	_featureB = 0;
	_featureC = 0;
	_featureD = 0;
	_featureHash = 0;
	memset(_extraState, 0, sizeof(_extraState));
	_stateDword = 0;
	_posX = 0;
	_posY = 0;
	_targetX = 0;
	_targetY = 0;
	_worldX = 0;
	_worldY = 0;
	_state34 = 0;
	_activeFlag = 0;
	_flagByte39 = 0;
	_freeStatus = 0;
	_sentinel = -1;
	_stateByte6C = 0;
	_zoombiniIndex = 0;
}

/**
 * Randomize features — corresponds to Zoombini__Zoombini_45BEE0.
 * Each feature is rand()%5+1 (values 1-5).
 */
void Zoombini::randomize(uint32 seed) {
	Common::RandomSource rnd(Common::String("zoombini_feature"));
	rnd.setSeed(seed);
	_featureA = (rnd.getRandomNumber(4)) + 1;
	_featureB = (rnd.getRandomNumber(4)) + 1;
	_featureC = (rnd.getRandomNumber(4)) + 1;
	_featureD = (rnd.getRandomNumber(4)) + 1;
	computeHash();
}

void Zoombini::setFeatures(byte a, byte b, byte c, byte d) {
	_featureA = a;
	_featureB = b;
	_featureC = c;
	_featureD = d;
	computeHash();
}

/**
 * Compute feature hash — from constructor decompilation.
 * hash = featureA + 8 * (featureB + 8 * (featureD + 8 * featureC))
 */
void Zoombini::computeHash() {
	_featureHash = _featureA + 8 * (_featureB + 8 * (_featureD + 8 * _featureC));
}

} // End of namespace Zoombini2
