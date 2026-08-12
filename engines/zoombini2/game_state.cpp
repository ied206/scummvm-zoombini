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

void BoardRecord::store(const Zoombini &zoombini) {
	memcpy(data, zoombini._extraState, 15);
	data[15] = zoombini._featureByte0;
	data[16] = zoombini._featureA;
	data[17] = zoombini._featureB;
	data[18] = zoombini._featureC;
	data[19] = zoombini._featureD;
}

Zoombini *BoardRecord::restore() const {
	Zoombini *zoombini = new Zoombini();
	memcpy(zoombini->_extraState, data, 15);
	zoombini->_featureByte0 = data[15];
	zoombini->setFeatures(data[16], data[17], data[18], data[19]);
	zoombini->_activeFlag = 1;
	zoombini->_zoombiniIndex = 33;
	return zoombini;
}

void GameState::clearBoard(BoardRecord **board) {
	for (int i = 0; i < kBoardRows * kBoardCols; i++) {
		delete board[i];
		board[i] = nullptr;
	}
}

bool GameState::storeInBoard(BoardRecord **board, Zoombini &zoombini) {
	int startRow = 62;
	for (int i = 0; i < kBoardRows * kBoardCols; i++) {
		if (board[i]) {
			startRow = i / kBoardCols;
			break;
		}
	}
	for (int direction = 1; -1 <= direction; direction -= 2) {
		for (int row = startRow; 0 <= row && row < kBoardRows; row += direction) {
			for (int col = 0; col < kBoardCols; col++) {
				BoardRecord *&cell = board[row * kBoardCols + col];
				if (!cell) {
					cell = new BoardRecord();
					cell->store(zoombini);
					zoombini._freeStatus = 0;
					return true;
				}
			}
		}
	}
	return false;
}

void GameState::refillFromBoard(BoardRecord **board, Common::Array<Zoombini *> &roster, uint count) {
	for (int col = 0; col < kBoardCols && roster.size() < count; col++) {
		for (int row = 0; row < kBoardRows && roster.size() < count; row++) {
			BoardRecord *&cell = board[row * kBoardCols + col];
			if (cell) {
				roster.push_back(cell->restore());
				delete cell;
				cell = nullptr;
			}
		}
	}
}

int GameState::findBoardScrollRow(BoardRecord *const *board) {
	for (int direction = 1; -1 <= direction; direction -= 2) {
		for (int col = 0; col < kBoardCols; col++) {
			for (int row = 62; 0 <= row && row < kBoardRows; row += direction) {
				if (board[row * kBoardCols + col])
					return kBoardRows < row + 6 ? 121 : row;
			}
		}
	}
	return 62;
}

int GameState::getFeatureCombinationTableIndex(byte featureA, byte featureB, byte featureC, byte featureD) {
	if (featureA < 1 || 5 < featureA || featureB < 1 || 5 < featureB || featureC < 1 || 5 < featureC || featureD < 1 || 5 < featureD)
		return -1;
	return (featureA - 1) + 5 * ((featureB - 1) + 5 * ((featureD - 1) + 5 * (featureC - 1)));
}

byte GameState::getScoreDataByte(int offset) const {
	if (offset < 0 || static_cast<int>(sizeof(_scoreData)) <= offset)
		return 0;
	const uint32 value = static_cast<uint32>(_scoreData[offset / 4]);
	return static_cast<byte>((value >> (8 * (offset % 4))) & 0xff);
}

void GameState::setScoreDataByte(int offset, byte value) {
	if (offset < 0 || static_cast<int>(sizeof(_scoreData)) <= offset)
		return;
	const int index = offset / 4;
	const int shift = 8 * (offset % 4);
	uint32 scoreValue = static_cast<uint32>(_scoreData[index]);
	scoreValue = (scoreValue & ~(0xffU << shift)) | (static_cast<uint32>(value) << shift);
	_scoreData[index] = static_cast<int32>(scoreValue);
}

bool GameState::canRegisterFeatures(byte featureA, byte featureB, byte featureC, byte featureD) const {
	static constexpr int kCombinationTableOffset = 12;
	const int tableIndex = getFeatureCombinationTableIndex(featureA, featureB, featureC, featureD);
	return 0 <= tableIndex && getScoreDataByte(kCombinationTableOffset + tableIndex) < 2;
}

bool GameState::registerFeatures(byte featureA, byte featureB, byte featureC, byte featureD) {
	static constexpr int kCombinationTableOffset = 12;
	const int tableIndex = getFeatureCombinationTableIndex(featureA, featureB, featureC, featureD);
	if (tableIndex < 0)
		return false;

	const int byteOffset = kCombinationTableOffset + tableIndex;
	const byte previousCount = getScoreDataByte(byteOffset);
	if (2 <= previousCount)
		return false;
	if (previousCount == 0)
		_scoreData[1] += 1;
	const byte count = previousCount + 1;
	setScoreDataByte(byteOffset, count);
	if (count == 2)
		_scoreData[2] += 1;
	_scoreData[0] += 1;
	return true;
}

GameState::GameState() : _boardA(), _boardB() {
	init();
}

GameState::~GameState() {
	clearOwnedData();
}

void GameState::clearOwnedData() {
	for (int i = 0; i < kBoardSize; i++) {
		delete _boardA[i];
		delete _boardB[i];
		_boardA[i] = nullptr;
		_boardB[i] = nullptr;
	}
	for (uint i = 0; i < _zoombinis.size(); i++)
		delete _zoombinis[i];
	_zoombinis.clear();
}

void GameState::init() {
	clearOwnedData();
	_playerName.clear();
	_gameMode = 0;
	_currentWorldId = 0;
	_flagByte1C = 0;
	_stateByteA = 0;
	_stateByteB = 0;
	_stateByteC = 0;
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
	_scoreData[0] = 1;
	_scoreData[1] = 1;
	_scoreData[156] = 1 << 24;
}

void GameState::swapState(GameState &other) {
	SWAP(_playerName, other._playerName);
	SWAP(_gameMode, other._gameMode);
	SWAP(_currentWorldId, other._currentWorldId);
	SWAP(_flagByte1C, other._flagByte1C);
	SWAP(_stateByteA, other._stateByteA);
	SWAP(_stateByteB, other._stateByteB);
	SWAP(_stateByteC, other._stateByteC);
	swapArray(_boardA, other._boardA);
	swapArray(_boardB, other._boardB);
	swapArray(_worldDataA, other._worldDataA);
	swapArray(_stateArray, other._stateArray);
	swapArray(_worldDataB, other._worldDataB);
	SWAP(_counterDword, other._counterDword);
	SWAP(_statA, other._statA);
	SWAP(_statB, other._statB);
	SWAP(_statC, other._statC);
	swapArray(_extendedState, other._extendedState);
	SWAP(_flagA, other._flagA);
	SWAP(_flagB, other._flagB);
	swapArray(_scoreData, other._scoreData);
	_zoombinis.swap(other._zoombinis);
}

bool GameState::canRead(Common::SeekableReadStream *stream, uint64 bytes) {
	if (!stream || stream->err() || stream->eos())
		return false;
	const int64 position = stream->pos();
	const int64 size = stream->size();
	return 0 <= position && position <= size && bytes <= static_cast<uint64>(size - position);
}

bool GameState::load(Common::SeekableReadStream *stream) {
	GameState loaded;
	if (!loaded.readState(stream))
		return false;
	swapState(loaded);
	return true;
}

void GameState::transferSavedRoster(Common::Array<Zoombini *> &source, Common::Array<Zoombini *> &destination) {
	assert(&source != &destination);
	for (uint i = 0; i < source.size(); i++) {
		const Zoombini *previous = source[i];
		Zoombini *restored = new Zoombini();
		restored->_featureByte0 = previous->_featureByte0;
		restored->setFeatures(previous->_featureA, previous->_featureB, previous->_featureC, previous->_featureD);
		memcpy(restored->_extraState, previous->_extraState, sizeof(restored->_extraState));
		destination.push_back(restored);
		delete previous;
	}
	source.clear();
}

bool GameState::readBoard(Common::SeekableReadStream *stream, BoardRecord **board) {
	if (!canRead(stream, 4))
		return false;
	const int32 count = stream->readSint32LE();
	if (count < 0 || kBoardRows * kBoardCols < count || !canRead(stream, static_cast<uint64>(count) * 28))
		return false;
	for (int32 i = 0; i < count; i++) {
		const int32 row = stream->readSint32LE();
		const int32 col = stream->readSint32LE();
		if (row < 0 || kBoardRows <= row || col < 0 || kBoardCols <= col)
			return false;
		const int index = row * kBoardCols + col;
		if (board[index])
			return false;
		board[index] = new BoardRecord();
		if (stream->read(board[index]->data, sizeof(board[index]->data)) != sizeof(board[index]->data))
			return false;
	}
	return !stream->err() && !stream->eos();
}

bool GameState::readState(Common::SeekableReadStream *stream) {
	if (!canRead(stream, 8) || stream->readSint32LE() != kSaveFileMagic)
		return false;
	const int32 nameLength = stream->readSint32LE();
	// A valid name includes its terminator and leaves room for every fixed field and count.
	if (nameLength < 1 || !canRead(stream, static_cast<uint64>(nameLength) + 2814))
		return false;
	Common::Array<char> name(nameLength);
	if (stream->read(name.data(), nameLength) != static_cast<uint32>(nameLength) || name[nameLength - 1] != '\0')
		return false;
	if (strlen(name.data()) != static_cast<uint32>(nameLength - 1))
		return false;
	_playerName = name.data();

	if (stream->read(_stateArray, sizeof(_stateArray)) != sizeof(_stateArray))
		return false;
	_counterDword = stream->readSint32LE();
	for (int i = 0; i < 100; i++)
		_worldDataA[i] = stream->readSint32LE();
	for (int i = 0; i < 100; i++)
		_worldDataB[i] = stream->readSint32LE();
	_stateByteA = stream->readByte();
	_stateByteB = stream->readByte();
	_stateByteC = stream->readByte();
	_flagByte1C = stream->readByte();
	_statA = stream->readSint32LE();
	_statB = stream->readSint32LE();
	_statC = stream->readSint32LE();
	for (int i = 0; i < 210; i++)
		_extendedState[i] = stream->readSint32LE();
	_flagA = stream->readByte();
	_flagB = stream->readByte();
	for (int i = 0; i < 160; i++)
		_scoreData[i] = stream->readSint32LE();

	if (!readBoard(stream, _boardA) || !readBoard(stream, _boardB) || !canRead(stream, 4))
		return false;
	const int32 count = stream->readSint32LE();
	if (count < 0 || !canRead(stream, static_cast<uint64>(count) * 20))
		return false;
	// The party is stored in two passes, unlike the interleaved sparse board records.
	for (int32 i = 0; i < count; i++) {
		Zoombini *zoombini = new Zoombini();
		_zoombinis.push_back(zoombini);
		zoombini->_featureByte0 = stream->readByte();
		zoombini->_featureA = stream->readByte();
		zoombini->_featureB = stream->readByte();
		zoombini->_featureC = stream->readByte();
		zoombini->_featureD = stream->readByte();
		zoombini->computeHash();
	}
	for (int32 i = 0; i < count; i++) {
		if (stream->read(_zoombinis[i]->_extraState, 15) != 15)
			return false;
	}
	return !stream->err() && !stream->eos();
}

int GameState::writeBoard(Common::WriteStream *stream, BoardRecord *const *board) {
	int count = 0;
	for (int i = 0; i < kBoardRows * kBoardCols; i++) {
		if (board[i])
			count += 1;
	}
	stream->writeSint32LE(count);
	for (int i = 0; i < kBoardRows * kBoardCols; i++) {
		if (board[i]) {
			stream->writeSint32LE(i / kBoardCols);
			stream->writeSint32LE(i % kBoardCols);
			stream->write(board[i]->data, sizeof(board[i]->data));
		}
	}
	return count;
}

bool GameState::save(Common::WriteStream *stream, const Common::Array<Zoombini *> *globalRoster) const {
	if (!stream || stream->err())
		return false;
	const uint32 globalCount = globalRoster && 0 <= _currentWorldId && _currentWorldId <= 3 ? globalRoster->size() : 0;
	const uint64 count = static_cast<uint64>(_zoombinis.size()) + globalCount;
	if (static_cast<uint64>(INT32_MAX) < count || static_cast<uint32>(INT32_MAX) <= _playerName.size())
		return false;
	const uint32 nameLength = _playerName.size() + 1;
	const int64 startPosition = stream->pos();
	stream->writeSint32LE(kSaveFileMagic);
	stream->writeUint32LE(nameLength);
	stream->write(_playerName.c_str(), nameLength);
	stream->write(_stateArray, sizeof(_stateArray));
	stream->writeSint32LE(_counterDword);
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_worldDataA[i]);
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_worldDataB[i]);
	stream->writeByte(_stateByteA);
	stream->writeByte(_stateByteB);
	stream->writeByte(_stateByteC);
	stream->writeByte(_flagByte1C);
	stream->writeSint32LE(_statA);
	stream->writeSint32LE(_statB);
	stream->writeSint32LE(_statC);
	for (int i = 0; i < 210; i++)
		stream->writeSint32LE(_extendedState[i]);
	stream->writeByte(_flagA);
	stream->writeByte(_flagB);
	for (int i = 0; i < 160; i++)
		stream->writeSint32LE(_scoreData[i]);
	const int boardACount = writeBoard(stream, _boardA);
	const int boardBCount = writeBoard(stream, _boardB);
	stream->writeUint32LE(static_cast<uint32>(count));
	for (uint64 i = 0; i < count; i++) {
		const Zoombini *zoombini = i < _zoombinis.size() ? _zoombinis[i] : (*globalRoster)[i - _zoombinis.size()];
		if (!zoombini)
			return false;
		stream->writeByte(zoombini->_featureByte0);
		stream->writeByte(zoombini->_featureA);
		stream->writeByte(zoombini->_featureB);
		stream->writeByte(zoombini->_featureC);
		stream->writeByte(zoombini->_featureD);
	}
	for (uint64 i = 0; i < count; i++) {
		const Zoombini *zoombini = i < _zoombinis.size() ? _zoombinis[i] : (*globalRoster)[i - _zoombinis.size()];
		stream->write(zoombini->_extraState, 15);
	}
	const int64 expectedSize = 2822 + static_cast<int64>(nameLength) + (boardACount + boardBCount) * 28 + count * 20;
	return !stream->err() && 0 <= startPosition && stream->pos() - startPosition == expectedSize;
}

void GameState::registerWorldVisit(int worldId, int visitKind) {
	if (worldId < 0 || 24 < worldId || visitKind < 1 || 3 < visitKind)
		return;
	byte &visits = _stateArray[5 * worldId + visitKind];
	if (visits < 250)
		visits += 1;
}

} // End of namespace Zoombini2
