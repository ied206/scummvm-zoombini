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

#ifndef ZOOMBINI2_GAME_STATE_H
#define ZOOMBINI2_GAME_STATE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/array.h"

namespace Zoombini2 {

class Zoombini;

/**
 * Save file version accepted by the loader.
 */
const int kSaveFileMagic = 262;

/**
 * Board grid dimensions.
 * There are 625 active entries and five unused sentinel cells.
 */
const int kBoardRows = 125;
const int kBoardCols = 5;
const int kBoardSize = 630; // includes sentinel row

/**
 * 20-byte board record stored per occupied cell.
 */
struct BoardRecord {
	byte data[20];

	BoardRecord() { memset(data, 0, sizeof(data)); }
	void store(const Zoombini &zoombini);
	Zoombini *restore() const;
};

/**
 * Stores game progress, zoombini roster, puzzle state, and world data.
 */
class GameState {
public:
	GameState();
	~GameState();

	void init();

	bool load(Common::SeekableReadStream *stream);
	bool save(Common::WriteStream *stream, const Common::Array<Zoombini *> *globalRoster = nullptr) const;
	static void transferSavedRoster(Common::Array<Zoombini *> &source, Common::Array<Zoombini *> &destination);
	static void clearBoard(BoardRecord **board);
	static bool storeInBoard(BoardRecord **board, Zoombini &zoombini);
	static void refillFromBoard(BoardRecord **board, Common::Array<Zoombini *> &roster, uint count);
	static int findBoardScrollRow(BoardRecord *const *board);
	bool canRegisterFeatures(byte featureA, byte featureB, byte featureC, byte featureD) const;
	bool registerFeatures(byte featureA, byte featureB, byte featureC, byte featureD);
	bool hasRegisteredEveryFeatureCombination() const { return _scoreData[0] == 625; }
	bool hasRelaxedPackFeatureLimits() const { return 600 <= _scoreData[2]; }

	/**
	 * Check if a world has been visited (any difficulty).
	 */
	bool isWorldVisited(int worldId) const {
		if (worldId < 0 || 100 <= worldId)
			return false;
		return _worldDataA[worldId] != 0;
	}

	/**
	 * Check one of the three visit counters belonging to a world.
	 */
	bool isWorldVisitedAtDiff(int worldId, int diff) const {
		if (diff < 1 || 3 < diff || worldId < 0 || 24 < worldId)
			return false;
		return _stateArray[5 * worldId + diff] != 0;
	}

	/**
	 * Increment a world's visit counter, saturating at 250.
	 */
	void registerWorldVisit(int worldId, int visitKind = 1);

	bool hasPlayedRescue1Movie() const { return _flagA != 0; }
	void markRescue1MoviePlayed() { _flagA = 1; }
	bool hasPlayedRescue2Movie() const { return _flagB != 0; }
	void markRescue2MoviePlayed() { _flagB = 1; }

	/**
	 * Get current difficulty level (1-4).
	 * The _gameMode field stores the difficulty when a game is active.
	 * Returns 1 (easy) by default if no difficulty is set.
	 */
	int getDifficulty() const {
		if (1 <= _gameMode && _gameMode <= 4)
			return _gameMode;
		return 1; // default to easy
	}

	// Player name stored in the save file.
	Common::String _playerName;

	// Runtime page difficulty and the last initialized gameplay world are independent.
	int _gameMode;
	int _currentWorldId;

	// State flags.
	byte _flagByte1C;
	byte _stateByteA;
	byte _stateByteB;
	byte _stateByteC;

	// Sparse boards, including the unused sentinel row.
	BoardRecord *_boardA[kBoardSize];

	BoardRecord *_boardB[kBoardSize];

	// World progress and byte-sized visit counters.
	int32 _worldDataA[100];

	byte _stateArray[500];

	int32 _worldDataB[100];

	// Counters and statistics.
	int32 _counterDword;

	int32 _statA;
	int32 _statB;
	int32 _statC;

	// Extended progress state.
	int32 _extendedState[210];

	// Rescue movie flags.
	byte _flagA;
	byte _flagB;

	// Score and rescue-stage data.
	int32 _scoreData[160];

	// Zoombini roster (local vector)
	Common::Array<Zoombini *> _zoombinis;

private:
	GameState(const GameState &) = delete;
	GameState &operator=(const GameState &) = delete;

	void clearOwnedData();
	void swapState(GameState &other);
	bool readState(Common::SeekableReadStream *stream);
	static bool canRead(Common::SeekableReadStream *stream, uint64 bytes);
	static bool readBoard(Common::SeekableReadStream *stream, BoardRecord **board);
	static int writeBoard(Common::WriteStream *stream, BoardRecord *const *board);
	static int getFeatureCombinationTableIndex(byte featureA, byte featureB, byte featureC, byte featureD);
	byte getScoreDataByte(int offset) const;
	void setScoreDataByte(int offset, byte value);
	template<typename T, uint N> static void swapArray(T (&a)[N], T (&b)[N]) {
		for (uint i = 0; i < N; i++)
			SWAP(a[i], b[i]);
	}
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GAME_STATE_H
