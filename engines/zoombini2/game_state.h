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
 * Save file magic version — must be 262 (0x106) or file is rejected.
 * Original: CGame+0x00.
 */
const int kSaveFileMagic = 262;

/**
 * Board grid dimensions.
 * Original: 125 rows x 5 columns = 625 active entries + 5 sentinel.
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
};

/**
 * CGame — main game state container (0x1EC0 = 7872 bytes in original).
 *
 * Stores game progress, zoombini roster, puzzle state, and world data.
 * Constructor: CGame__CGame_458280 (0x458280)
 * Init/Reset:  CGame__Init_458330 (0x458330)
 * Load:        CGame__Load_458770 (0x458770)
 * Save:        CGame__Save_4592D0 (0x4592D0)
 */
class GameState {
public:
	GameState();
	~GameState();

	void init();

	bool load(Common::SeekableReadStream *stream);
	bool save(Common::WriteStream *stream);

	/**
	 * Check if a world has been visited (any difficulty).
	 * Original: Buffer[1268 + worldId] != 0.
	 */
	bool isWorldVisited(int worldId) const {
		if (worldId < 0 || worldId >= 100)
			return false;
		return _worldDataA[worldId] != 0;
	}

	/**
	 * Check if a world was visited at a specific difficulty level (1-3).
	 * Original: CGame__IsWorldVisitedAtDiff_459BC0.
	 * Accesses byte at offset (5 * worldId + diff) within _stateArray.
	 */
	bool isWorldVisitedAtDiff(int worldId, int diff) const {
		if (diff < 1 || diff > 3 || worldId < 0 || worldId > 24)
			return false;
		const byte *raw = reinterpret_cast<const byte *>(_stateArray);
		return raw[5 * worldId + diff] != 0;
	}

	/**
	 * Register that a world (puzzle) has been visited.
	 * Original: Scene__RegisterHandler_459B80.
	 * Increments byte at _stateArray[5*worldId + 1] (hardcodes diff=1).
	 */
	void registerWorldVisit(int worldId);

	bool hasPlayedRescue1Movie() const { return _flagA != 0; }
	void markRescue1MoviePlayed() { _flagA = 1; }
	bool hasPlayedRescue2Movie() const { return _flagB != 0; }
	void markRescue2MoviePlayed() { _flagB = 1; }

	/**
	 * Get current difficulty level (1=easy, 2=medium, 3=hard).
	 * The _gameMode field stores the difficulty when a game is active.
	 * Returns 1 (easy) by default if no difficulty is set.
	 */
	int getDifficulty() const {
		if (_gameMode >= 1 && _gameMode <= 3)
			return _gameMode;
		return 1; // default to easy
	}

	// +0x04: Player name
	Common::String _playerName;

	// +0x08: Game mode (0=menu?, 1/2/3=difficulty?)
	int _gameMode;

	// +0x1C-0x1F: State flags
	byte _flagByte1C;
	byte _stateByteA;  // +0x1D
	byte _stateByteB;  // +0x1E
	byte _stateByteC;  // +0x1F

	// +0x20: Board A — 125×5 grid of board records
	BoardRecord *_boardA[kBoardSize];

	// +0x9F8: Board B — same layout
	BoardRecord *_boardB[kBoardSize];

	// +0x13D0: World data A — 100 DWORDs
	int32 _worldDataA[100];

	// +0x1560: State array — 125 DWORDs
	int32 _stateArray[125];

	// +0x1754: World data B — 100 DWORDs
	int32 _worldDataB[100];

	// +0x18E4: Counter
	int32 _counterDword;

	// +0x18E8-0x18F0: Statistics
	int32 _statA;
	int32 _statB;
	int32 _statC;

	// +0x18F4: Extended state — 210 DWORDs
	int32 _extendedState[210];

	// +0x1C3C-0x1C3D: Flags
	byte _flagA;
	byte _flagB;

	// +0x1C40: Score data — 160 DWORDs
	int32 _scoreData[160];

	// Zoombini roster (local vector)
	Common::Array<Zoombini *> _zoombinis;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GAME_STATE_H
