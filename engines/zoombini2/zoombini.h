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

#ifndef ZOOMBINI2_ZOOMBINI_H
#define ZOOMBINI2_ZOOMBINI_H

#include "common/scummsys.h"

namespace Zoombini2 {

/**
 * Zoombini character object — 0x9C (156) bytes in original.
 *
 * Constructor: Zoombini__Zoombini_45BEE0
 * Each zoombini has 4 features with values 1-5.
 * Feature hash: featureA + 8 * (featureB + 8 * (featureD + 8 * featureC))
 *             = featureA + 8*featureB + 64*featureD + 512*featureC
 */
class Zoombini {
public:
	Zoombini();
	~Zoombini() {}

	void randomize(uint32 seed);
	void setFeatures(byte a, byte b, byte c, byte d);
	void computeHash();

	// +0x00: Status/type DWORD
	int32 _status;

	// +0x04: Feature byte 0 (sub-record, not set in ctor)
	byte _featureByte0;

	// +0x05-0x08: Feature values (1-5 each)
	byte _featureA;  // Hair
	byte _featureB;  // Eyes
	byte _featureC;  // Nose
	byte _featureD;  // Feet

	// +0x0A: Feature hash
	uint16 _featureHash;

	// +0x0C: Serialized extra state (15 bytes)
	byte _extraState[15];

	// +0x1C: State DWORD
	int32 _stateDword;

	// +0x20-0x24: Screen position
	int32 _posX;
	int32 _posY;

	// +0x28-0x2C: Movement target
	int32 _targetX;
	int32 _targetY;

	// +0x30-0x32: World grid position
	int16 _worldX;
	int16 _worldY;

	// +0x34: State DWORD
	int32 _state34;

	// +0x38-0x39: Active flags
	byte _activeFlag;
	byte _flagByte39;

	// +0x5C: Free status — 0 = "free" zoombini
	byte _freeStatus;

	// +0x68: Sentinel — initialized to -1
	int32 _sentinel;

	// +0x6C: State byte
	byte _stateByte6C;

	// +0x70: Index in world data
	int32 _zoombiniIndex;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_ZOOMBINI_H
