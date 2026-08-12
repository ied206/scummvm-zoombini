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

#include "zoombini2/game_state.h"
#include "zoombini2/zoombini.h"

namespace Zoombini2 {

GameState::GameState() {
	init();
}

GameState::~GameState() {
	for (int i = 0; i < kBoardSize; i++) {
		delete _boardA[i];
		delete _boardB[i];
	}
	for (uint i = 0; i < _zoombinis.size(); i++) {
		delete _zoombinis[i];
	}
}

/**
 * Initialize/reset all mutable state fields — corresponds to CGame__Init_458330.
 */
void GameState::init() {
	_playerName.clear();
	_gameMode = 0;

	_flagByte1C = 0;
	_stateByteA = 0;
	_stateByteB = 0;
	_stateByteC = 0;

	for (int i = 0; i < kBoardSize; i++) {
		_boardA[i] = nullptr;
		_boardB[i] = nullptr;
	}

	memset(_worldDataA, 0, sizeof(_worldDataA));
	memset(_stateArray, 0, sizeof(_stateArray));
	memset(_worldDataB, 0, sizeof(_worldDataB));

	_counterDword = 0;
	_statA = 0;
	_statB = 0;
	_statC = 0;

	memset(_extendedState, 0, sizeof(_extendedState));

	_flagA = 0;
	_flagB = 0;

	memset(_scoreData, 0, sizeof(_scoreData));
}

/**
 * Load game state from a .mk save file.
 * Serialization order matches CGame__Load_458770.
 */
bool GameState::load(Common::SeekableReadStream *stream) {
	int32 magic = stream->readSint32LE();
	if (magic != kSaveFileMagic)
		return false;

	// Player name: length-prefixed string
	int32 nameLen = stream->readSint32LE();
	if (nameLen > 0 && nameLen < 256) {
		char buf[256];
		stream->read(buf, nameLen);
		buf[nameLen] = '\0';
		_playerName = buf;
	}

	// State array (500 bytes = 125 DWORDs)
	for (int i = 0; i < 125; i++)
		_stateArray[i] = stream->readSint32LE();

	// Counter
	_counterDword = stream->readSint32LE();

	// World data A (400 bytes = 100 DWORDs)
	for (int i = 0; i < 100; i++)
		_worldDataA[i] = stream->readSint32LE();

	// World data B (400 bytes = 100 DWORDs)
	for (int i = 0; i < 100; i++)
		_worldDataB[i] = stream->readSint32LE();

	// State bytes
	_stateByteA = stream->readByte();
	_stateByteB = stream->readByte();
	_stateByteC = stream->readByte();
	_flagByte1C = stream->readByte();

	// Statistics
	_statA = stream->readSint32LE();
	_statB = stream->readSint32LE();
	_statC = stream->readSint32LE();

	// Extended state (840 bytes = 210 DWORDs)
	for (int i = 0; i < 210; i++)
		_extendedState[i] = stream->readSint32LE();

	// Flags
	_flagA = stream->readByte();
	_flagB = stream->readByte();

	// Score data (640 bytes = 160 DWORDs)
	for (int i = 0; i < 160; i++)
		_scoreData[i] = stream->readSint32LE();

	// Board A entries
	int32 boardACount = stream->readSint32LE();
	for (int32 i = 0; i < boardACount; i++) {
		int32 row = stream->readSint32LE();
		int32 col = stream->readSint32LE();
		int idx = row * kBoardCols + col;
		if (idx >= 0 && idx < kBoardSize) {
			_boardA[idx] = new BoardRecord();
			stream->read(_boardA[idx]->data, 20);
		} else {
			// Skip invalid entry
			stream->skip(20);
		}
	}

	// Board B entries
	int32 boardBCount = stream->readSint32LE();
	for (int32 i = 0; i < boardBCount; i++) {
		int32 row = stream->readSint32LE();
		int32 col = stream->readSint32LE();
		int idx = row * kBoardCols + col;
		if (idx >= 0 && idx < kBoardSize) {
			_boardB[idx] = new BoardRecord();
			stream->read(_boardB[idx]->data, 20);
		} else {
			stream->skip(20);
		}
	}

	// Zoombini roster
	int32 zoombiniCount = stream->readSint32LE();
	_zoombinis.clear();
	// Features pass
	Common::Array<byte> featureData(zoombiniCount * 5);
	stream->read(featureData.data(), zoombiniCount * 5);
	// Extra state pass
	Common::Array<byte> extraData(zoombiniCount * 15);
	stream->read(extraData.data(), zoombiniCount * 15);

	for (int32 i = 0; i < zoombiniCount; i++) {
		Zoombini *z = new Zoombini();
		z->_featureByte0 = featureData[i * 5 + 0];
		z->_featureA     = featureData[i * 5 + 1];
		z->_featureB     = featureData[i * 5 + 2];
		z->_featureC     = featureData[i * 5 + 3];
		z->_featureD     = featureData[i * 5 + 4];
		z->computeHash();
		memcpy(z->_extraState, &extraData[i * 15], 15);
		_zoombinis.push_back(z);
	}

	return true;
}

/**
 * Save game state to a .mk save file.
 * Serialization order matches CGame__Save_4592D0.
 */
bool GameState::save(Common::WriteStream *stream) {
	stream->writeSint32LE(kSaveFileMagic);

	// Player name
	int32 nameLen = _playerName.size() + 1;
	stream->writeSint32LE(nameLen);
	stream->write(_playerName.c_str(), nameLen);

	// State array
	for (int i = 0; i < 125; i++)
		stream->writeSint32LE(_stateArray[i]);

	// Counter
	stream->writeSint32LE(_counterDword);

	// World data A
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_worldDataA[i]);

	// World data B
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_worldDataB[i]);

	// State bytes
	stream->writeByte(_stateByteA);
	stream->writeByte(_stateByteB);
	stream->writeByte(_stateByteC);
	stream->writeByte(_flagByte1C);

	// Statistics
	stream->writeSint32LE(_statA);
	stream->writeSint32LE(_statB);
	stream->writeSint32LE(_statC);

	// Extended state
	for (int i = 0; i < 210; i++)
		stream->writeSint32LE(_extendedState[i]);

	// Flags
	stream->writeByte(_flagA);
	stream->writeByte(_flagB);

	// Score data
	for (int i = 0; i < 160; i++)
		stream->writeSint32LE(_scoreData[i]);

	// Board A
	int32 boardACount = 0;
	for (int i = 0; i < kBoardSize; i++) {
		if (_boardA[i])
			boardACount++;
	}
	stream->writeSint32LE(boardACount);
	for (int i = 0; i < kBoardSize; i++) {
		if (_boardA[i]) {
			stream->writeSint32LE(i / kBoardCols); // row
			stream->writeSint32LE(i % kBoardCols); // col
			stream->write(_boardA[i]->data, 20);
		}
	}

	// Board B
	int32 boardBCount = 0;
	for (int i = 0; i < kBoardSize; i++) {
		if (_boardB[i])
			boardBCount++;
	}
	stream->writeSint32LE(boardBCount);
	for (int i = 0; i < kBoardSize; i++) {
		if (_boardB[i]) {
			stream->writeSint32LE(i / kBoardCols);
			stream->writeSint32LE(i % kBoardCols);
			stream->write(_boardB[i]->data, 20);
		}
	}

	// Zoombini roster
	int32 zoombiniCount = _zoombinis.size();
	stream->writeSint32LE(zoombiniCount);
	// Features pass
	for (int32 i = 0; i < zoombiniCount; i++) {
		stream->writeByte(_zoombinis[i]->_featureByte0);
		stream->writeByte(_zoombinis[i]->_featureA);
		stream->writeByte(_zoombinis[i]->_featureB);
		stream->writeByte(_zoombinis[i]->_featureC);
		stream->writeByte(_zoombinis[i]->_featureD);
	}
	// Extra state pass
	for (int32 i = 0; i < zoombiniCount; i++) {
		stream->write(_zoombinis[i]->_extraState, 15);
	}

	return true;
}

void GameState::registerWorldVisit(int worldId) {
	if (worldId < 0 || worldId > 24)
		return;
	byte *raw = reinterpret_cast<byte *>(_stateArray);
	// Original hardcodes diff=1 (Scene__RegisterHandler_459B80)
	raw[5 * worldId + 1]++;
}

} // End of namespace Zoombini2
