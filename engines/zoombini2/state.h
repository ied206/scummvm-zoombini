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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_STATE_H
#define ZOOMBINI2_STATE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str-array.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {
class SaveFileManager;
}

namespace Zoombini2 {

/** Save-file version accepted by @ref GameState. */
const int kSaveFileMagic = 262;

/** Number of serialized rows in each sparse storage board. */
const int kBoardRows = 125;

/** Number of serialized columns in each sparse storage board. */
const int kBoardCols = 5;

/** Board allocation size, including one unused sentinel row. */
const int kBoardSize = 630;

/**
 * Runtime and serialized state for one Zoombini.
 *
 * The four feature values use the inclusive range 1 through 5.
 * @ref ZoombiniState::_featureHash encodes them as `D + 8 * (C + 8 * (B + 8 * A))`.
 * Save files persist the five feature bytes and the fifteen-byte
 * @ref ZoombiniState::_extraState block; movement and drawing fields remain runtime-only.
 */
class ZoombiniState {
public:
	/** Initialize every field to the inactive runtime baseline. */
	ZoombiniState();
	/** Destroy the runtime state. */
	~ZoombiniState() {}

	/** Select four deterministic feature values from a local seeded generator. */
	void randomize(uint32 seed);
	/** Assign the four feature values and update @ref ZoombiniState::_featureHash. */
	void setFeatures(byte featureA, byte featureB, byte featureC, byte featureD);
	/** Recompute @ref ZoombiniState::_featureHash from the current feature values. */
	void computeHash();
	/** Return the packed hash for four feature values. */
	static uint16 calculateFeatureHash(byte featureA, byte featureB, byte featureC, byte featureD);

	/** Runtime status or activity code. */
	int32 _status;
	/** Serialized status byte that precedes the four visible traits. */
	byte _featureByte0;
	/** Feet trait value in the inclusive range 1 through 5. */
	byte _featureA;
	/** Nose trait value in the inclusive range 1 through 5. */
	byte _featureB;
	/** Hair trait value in the inclusive range 1 through 5. */
	byte _featureC;
	/** Eyes trait value in the inclusive range 1 through 5. */
	byte _featureD;
	/** Packed value derived from @ref ZoombiniState::_featureA through @ref ZoombiniState::_featureD. */
	uint16 _featureHash;
	/** Fifteen bytes of state retained by boards and profile saves. */
	byte _extraState[15];
	/** Runtime activity state used by puzzle and transition code. */
	int32 _stateDword;
	/** Current signed 32-bit screen position. */
	Common::Point32 _position;
	/** Signed 32-bit destination for the active movement. */
	Common::Point32 _targetPosition;
	/** Horizontal position in world-grid coordinates. */
	int16 _worldX;
	/** Vertical position in world-grid coordinates. */
	int16 _worldY;
	/** Additional runtime activity state. */
	int32 _state34;
	/** Whether the Zoombini is active in its current scene. */
	byte _activeFlag;
	/** Secondary runtime activity flag. */
	byte _flagByte39;
	/** Availability state, where zero denotes a free Zoombini. */
	byte _freeStatus;
	/** Runtime sentinel initialized to -1. */
	int32 _sentinel;
	/** Additional byte-sized runtime state. */
	byte _stateByte6C;
	/** Index used by world and animation data. */
	int32 _zoombiniIndex;
};

/** Twenty-byte representation of one Zoombini stored in a sparse board cell. */
struct BoardRecord {
	/** Serialized extra-state bytes followed by the five feature bytes. */
	byte data[20];

	/** Initialize the record to an empty byte sequence. */
	BoardRecord() { memset(data, 0, sizeof(data)); }
	/** Copy the persistent fields from @p zoombini into this record. */
	void store(const ZoombiniState &zoombini);
	/** Restore a newly allocated active Zoombini from this record. */
	ZoombiniState *restore() const;
};

/**
 * Owns one profile's progress, local roster, sparse boards, and score state.
 *
 * Loads are transactional: a complete temporary state is validated before it
 * replaces the active instance. The class also owns every pointer stored in
 * @ref GameState::_boardA, @ref GameState::_boardB, and @ref GameState::_zoombinis.
 */
class GameState {
public:
	/** Construct a fresh profile state. */
	GameState();
	/** Release all board records and local-roster entries. */
	~GameState();

	/** Reset all serialized and runtime profile fields to their initial values. */
	void init();

	/** Load and validate a complete profile from @p stream. */
	bool load(Common::SeekableReadStream *stream);
	/** Serialize this profile and an optional engine-owned roster to @p stream. */
	bool save(Common::WriteStream *stream, const Common::Array<ZoombiniState *> *globalRoster = nullptr) const;
	/** Move saved Zoombini state from @p source into newly allocated entries in @p destination. */
	static void transferSavedRoster(Common::Array<ZoombiniState *> &source, Common::Array<ZoombiniState *> &destination);
	/** Delete every record in @p board and clear its cells. */
	static void clearBoard(BoardRecord **board);
	/** Store @p zoombini in the first available cell of @p board. */
	static bool storeInBoard(BoardRecord **board, ZoombiniState &zoombini);
	/** Restore up to @p count Zoombinis from @p board into @p roster. */
	static void refillFromBoard(BoardRecord **board, Common::Array<ZoombiniState *> &roster, uint count);
	/** Return the board row used to initialize the shelter scroll position. */
	static int findBoardScrollRow(BoardRecord *const *board);
	/** Return whether one more copy of the supplied feature combination may be registered. */
	bool canRegisterFeatures(byte featureA, byte featureB, byte featureC, byte featureD) const;
	/** Register one feature combination if its per-combination limit has not been reached. */
	bool registerFeatures(byte featureA, byte featureB, byte featureC, byte featureD);
	/** Return whether all 625 feature combinations have been registered. */
	bool hasRegisteredEveryFeatureCombination() const { return _scoreData[0] == 625; }
	/** Return whether accumulated progress has unlocked relaxed party feature limits. */
	bool hasRelaxedPackFeatureLimits() const { return 600 <= _scoreData[2]; }

	/** Return whether @p worldId has any recorded visit. */
	bool isWorldVisited(int worldId) const {
		if (worldId < 0 || 100 <= worldId)
			return false;
		return _worldDataA[worldId] != 0;
	}

	/** Return whether @p worldId has a visit recorded for @p difficulty. */
	bool isWorldVisitedAtDiff(int worldId, int difficulty) const {
		if (difficulty < 1 || 3 < difficulty || worldId < 0 || 24 < worldId)
			return false;
		return _stateArray[5 * worldId + difficulty] != 0;
	}

	/** Increment a world's selected visit counter, saturating at 250. */
	void registerWorldVisit(int worldId, int visitKind = 1);

	/** Return whether the first rescue movie has already played. */
	bool hasPlayedRescue1Movie() const { return _flagA != 0; }
	/** Record that the first rescue movie has played. */
	void markRescue1MoviePlayed() { _flagA = 1; }
	/** Return whether the second rescue movie has already played. */
	bool hasPlayedRescue2Movie() const { return _flagB != 0; }
	/** Record that the second rescue movie has played. */
	void markRescue2MoviePlayed() { _flagB = 1; }

	/** Return the active difficulty in the inclusive range 1 through 4, defaulting to 1. */
	int getDifficulty() const {
		if (1 <= _gameMode && _gameMode <= 4)
			return _gameMode;
		return 1;
	}

	/** Player-visible profile name stored in the save file. */
	Common::String _playerName;
	/** Runtime difficulty selected for the active gameplay page. */
	int _gameMode;
	/** Most recently initialized gameplay world. */
	int _currentWorldId;
	/** General byte-sized profile flag. */
	byte _flagByte1C;
	/** First byte-sized gameplay state value. */
	byte _stateByteA;
	/** Second byte-sized gameplay state value. */
	byte _stateByteB;
	/** Third byte-sized gameplay state value. */
	byte _stateByteC;
	/** First sparse storage board, including its unused sentinel row. */
	BoardRecord *_boardA[kBoardSize];
	/** Second sparse storage board, including its unused sentinel row. */
	BoardRecord *_boardB[kBoardSize];
	/** Per-world progress values. */
	int32 _worldDataA[100];
	/** Byte-sized per-world visit counters and gameplay state. */
	byte _stateArray[500];
	/** Secondary per-world progress values. */
	int32 _worldDataB[100];
	/** General progress counter. */
	int32 _counterDword;
	/** First profile statistic. */
	int32 _statA;
	/** Second profile statistic. */
	int32 _statB;
	/** Third profile statistic and used-prefix length for @ref GameState::_extendedState. */
	int32 _statC;
	/** Extended progress records. */
	int32 _extendedState[210];
	/** First-rescue movie completion flag. */
	byte _flagA;
	/** Second-rescue movie completion flag. */
	byte _flagB;
	/** Feature-combination counts, aggregate progress, and rescue-stage data. */
	int32 _scoreData[160];
	/** Profile-owned Zoombini roster. */
	Common::Array<ZoombiniState *> _zoombinis;

private:
	/** Disallow copying profile-owned pointers. */
	GameState(const GameState &) = delete;
	/** Disallow assigning profile-owned pointers. */
	GameState &operator=(const GameState &) = delete;

	/** Delete all owned board and roster entries. */
	void clearOwnedData();
	/** Exchange all owned and scalar state with @p other. */
	void swapState(GameState &other);
	/** Parse the complete state body from @p stream. */
	bool readState(Common::SeekableReadStream *stream);
	/** Return whether @p bytes can be read from the stream's current position. */
	static bool canRead(Common::SeekableReadStream *stream, uint64 bytes);
	/** Parse one sparse board from @p stream. */
	static bool readBoard(Common::SeekableReadStream *stream, BoardRecord **board);
	/** Serialize one sparse board and return its occupied-cell count. */
	static int writeBoard(Common::WriteStream *stream, BoardRecord *const *board);
	/** Return the counter-table index for a validated feature combination. */
	static int getFeatureCombinationTableIndex(byte featureA, byte featureB, byte featureC, byte featureD);
	/** Read one logical byte from @ref GameState::_scoreData. */
	byte getScoreDataByte(int offset) const;
	/** Replace one logical byte in @ref GameState::_scoreData. */
	void setScoreDataByte(int offset, byte value);
	/** Exchange two fixed-size arrays element by element. */
	template<typename T, uint N>
	static void swapArray(T (&a)[N], T (&b)[N]) {
		for (uint i = 0; i < N; i++)
			SWAP(a[i], b[i]);
	}
};

/**
 * Target-scoped storage for named player profiles.
 *
 * The manager validates portable profile names, serializes @ref GameState, and
 * verifies newly written bytes before reporting success.
 */
class Zoombini2SavegameManager {
public:
	/** Maximum number of characters accepted in a profile name. */
	static constexpr int kMaximumProfileNameLength = 16;

	/** Bind profile operations to @p saveFileManager and @p target. */
	Zoombini2SavegameManager(Common::SaveFileManager *saveFileManager, const Common::String &target);

	/** Return valid profile names in case-insensitive sort order. */
	Common::StringArray listProfiles() const;
	/** Serialize @p state to @p profileName with an optional engine-owned roster. */
	bool saveProfile(const Common::String &profileName, const GameState &state, const Common::Array<ZoombiniState *> *globalRoster = nullptr) const;
	/** Load @p profileName transactionally into @p state. */
	bool loadProfile(const Common::String &profileName, GameState &state) const;
	/** Delete the save belonging to @p profileName. */
	bool deleteProfile(const Common::String &profileName) const;
	/** Rename a profile without overwriting another profile. */
	bool renameProfile(const Common::String &oldProfileName, const Common::String &newProfileName) const;
	/** Return whether a valid profile has a target-scoped save file. */
	bool profileExists(const Common::String &profileName) const;

	/** Return whether @p profileName is portable and valid for the profile UI. */
	static bool isValidProfileName(const Common::String &profileName);

private:
	/** Build the target-scoped filename for @p profileName. */
	Common::String makeSaveFileName(const Common::String &profileName) const;
	/** Insert @p profileName into @p profiles unless it is already present. */
	static void addProfileSorted(Common::StringArray &profiles, const Common::String &profileName);
	/** Verify that a completed save contains exactly @p size bytes from @p data. */
	bool verifySaveData(const Common::String &saveFileName, const byte *data, uint32 size) const;

	/** Backend that owns target save streams. */
	Common::SaveFileManager *_saveFileManager;
	/** ScummVM target identifier used as the save-file namespace. */
	Common::String _target;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_STATE_H
