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
#include "common/scummsys.h"
#include "common/str-array.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {
class SaveFileManager;
} // namespace Common

namespace Zoombini2 {

class ZoombiniState;

/** Save-file version accepted by @ref GameState. */
const int kSaveFileMagic = 262;

/** Number of serialized rows in each sparse storage board. */
const int kBoardRows = 125;

/** Number of serialized columns in each sparse storage board. */
const int kBoardCols = 5;

/** Board allocation size, including one unused sentinel row. */
const int kBoardSize = 630;

/** Number of distinct Zoombinis represented by the four visible traits. */
const int kZoombiniCombinationCount = 625;

/** Bytes reserved for a NUL-terminated Zoombini name in profile saves. */
const int kZoombiniNameSize = 15;

/** Number of completed-Zoombini trait hashes retained in one profile. */
const int kCompletedTraitHashCount = 210;

/** Stored Zoombini traits with an unused slot zero followed by the four visible values. */
struct ZmbTrait {
	/** Number of visible traits in the tuple. */
	static constexpr int kTraitCount = 4;
	/** Number of selectable values for each visible trait. */
	static constexpr int kTraitValueCount = 5;

	/** Identity and tuple index of one visible trait. */
	enum class TraitIndex : byte {
		/** Feet at tuple index zero. */
		kFeet00 = 0,
		/** Nose at tuple index one. */
		kNose01 = 1,
		/** Hair at tuple index two. */
		kHair02 = 2,
		/** Eyes at tuple index three. */
		kEyes03 = 3
	};
	/** Initialize the unused slot and every trait to zero. */
	ZmbTrait() = default;
	/** Initialize the complete visible appearance. */
	ZmbTrait(byte feet, byte nose, byte hair, byte eyes) : _feet(feet), _nose(nose), _hair(hair), _eyes(eyes) {}

	/** Return the trait identified by @p index. */
	byte getValue(TraitIndex index) const;
	/** Return whether every trait is in the inclusive range 1 through 5. */
	bool hasValidValues() const;
	/** Return the packed identifier derived from all four traits. */
	uint16 calculateHash() const;
	/** Decode a packed identifier into feet, nose, hair, eyes storage order. */
	static ZmbTrait fromHash(uint16 traitHash);
	/** Return the ScummVM-internal diagnostic name for one visible trait value. */
	static const char *debugTraitValueName(TraitIndex index, int value) {
		static constexpr const char *kZoombiniTraitNames[kTraitCount][kTraitValueCount] = {
			{"Sneakers", "RollerSkates", "Spring", "Wheels", "Propeller"},
			{"Orange", "White", "Green", "Blue", "Pink"},
			{"Braids", "LongHair", "BaseballCap", "Ponytail", "SpikyHair"},
			{"NormalEyed", "OneEyed", "SleepyEyed", "Glasses", "Sunglasses"}};
		const int traitOrdinal = static_cast<int>(index);
		if (0 <= traitOrdinal && traitOrdinal < kTraitCount && 1 <= value && value <= kTraitValueCount)
			return kZoombiniTraitNames[traitOrdinal][value - 1];
		return "?";
	}
	/** Return ScummVM-internal trait names in the Z1 diagnostic output pattern. */
	Common::String toStr() const {
		return Common::String::format("%s, %s, %s, %s",
									  debugTraitValueName(TraitIndex::kHair02, _hair),
									  debugTraitValueName(TraitIndex::kEyes03, _eyes),
									  debugTraitValueName(TraitIndex::kNose01, _nose),
									  debugTraitValueName(TraitIndex::kFeet00, _feet));
	}
	/** Return whether every trait equals the corresponding value in @p other. */
	bool operator==(const ZmbTrait &other) const {
		return _feet == other._feet && _nose == other._nose && _hair == other._hair && _eyes == other._eyes;
	}
	/** Return whether any trait differs from the corresponding value in @p other. */
	bool operator!=(const ZmbTrait &other) const { return !(*this == other); }

	/** Serialized but unused element zero of the original one-based trait storage. */
	byte _unusedSlot0 = 0;
	/** Feet trait value in the inclusive range 1 through 5. */
	byte _feet = 0;
	/** Nose trait value in the inclusive range 1 through 5. */
	byte _nose = 0;
	/** Hair trait value in the inclusive range 1 through 5. */
	byte _hair = 0;
	/** Eyes trait value in the inclusive range 1 through 5. */
	byte _eyes = 0;
};

/** Population counts displayed for one saved profile. */
struct Zoombini2PopulationSummary {
	Zoombini2PopulationSummary() = default;

	/** Zoombinis not yet recorded at either rescue site or Booliewood. */
	int _zombinivilleCount = 0;
	/** Zoombinis stored at Rescue Site I. */
	int _rescue1Count = 0;
	/** Zoombinis stored at Rescue Site II. */
	int _rescue2Count = 0;
	/** Zoombinis recorded as having reached Booliewood. */
	int _booliewoodCount = 0;
	/** Zoombinis serialized in the saved active party. */
	int _activePartyCount = 0;
};

/** Name, validation state, and population counts for one independent profile save. */
struct Zoombini2ProfileSummary {
	Zoombini2ProfileSummary() = default;

	/** Profile name derived from the target-scoped .mk filename. */
	Common::String _profileName;
	/** Whether the complete .mk stream could be parsed. */
	bool _stateValid = false;
	/** Counts parsed from the independent .mk stream when @ref Zoombini2ProfileSummary::_stateValid is true. */
	Zoombini2PopulationSummary _population;
};

/** Twenty-byte representation of one Zoombini stored in a sparse board cell. */
struct BoardRecord {
	/** NUL-terminated character name copied from the active Zoombini. */
	char _name[kZoombiniNameSize] = {};
	/** Unused slot zero followed by Feet, Nose, Hair, and Eyes. */
	ZmbTrait _traits;

	/** Initialize the record to an empty name and unset stored traits. */
	BoardRecord() = default;
	/** Copy the persistent fields from @p zoombini into this record. */
	void store(const ZoombiniState &zoombini);
	/** Return the complete trait tuple stored in this record. */
	ZmbTrait getTraits() const;
	/** Restore a newly allocated active Zoombini from this record. */
	ZoombiniState *restore() const;
};

/** Aggregate counters and per-combination usage retained by the profile. */
struct TraitRegistrationState {
	/** Total accepted registrations, including repeated combinations. */
	int32 _totalCount;
	/** Number of distinct trait combinations registered at least once. */
	int32 _uniqueCombinationCount;
	/** Number of trait combinations registered exactly twice. */
	int32 _twiceRegisteredCombinationCount;
	/** Registration count for each of the 625 visible trait combinations. */
	byte _combinationUseCounts[kZoombiniCombinationCount];
	/** Unused trailing bytes preserved by the original 640-byte save block. */
	byte _unusedTail[3];
};

/**
 * Owns one profile's progress, local roster, sparse boards, and trait-registration state.
 *
 * Loads are transactional: a complete temporary state is validated before it
 * replaces the active instance. The class also owns every pointer stored in
 * @ref GameState::_rescue1Board, @ref GameState::_rescue2Board, and
 * @ref GameState::_savedRoster.
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
	/** Serialize this profile and, when supplied, the active party roster to @p stream. */
	bool save(Common::WriteStream *stream, const Common::Array<ZoombiniState *> *globalRoster = nullptr) const;
	/** Move saved Zoombini state from @p src into newly allocated entries in @p dest. */
	static void transferSavedRoster(Common::Array<ZoombiniState *> &src, Common::Array<ZoombiniState *> &dest);
	/** Delete every record in @p board and clear its cells. */
	static void clearBoard(BoardRecord **board);
	/** Store @p zoombini in the first available cell of @p board. */
	static bool storeInBoard(BoardRecord **board, ZoombiniState &zoombini);
	/** Restore up to @p count Zoombinis from @p board into @p roster. */
	static void refillFromBoard(BoardRecord **board, Common::Array<ZoombiniState *> &roster, uint count);
	/** Return the board row used to initialize the shelter scroll position. */
	static int findBoardScrollRow(BoardRecord *const *board);
	/** Return the four shelter-stage counts and serialized active-party count. */
	Zoombini2PopulationSummary getPopulationSummary() const;
	/** Return the registration count for @p traits, or zero for invalid values. */
	byte getTraitCombinationRegistrationCount(const ZmbTrait &traits) const;
	/** Return whether one more copy of the supplied trait combination may be registered. */
	bool canRegisterTraits(const ZmbTrait &traits) const;
	/** Register one trait combination if its per-combination limit has not been reached. */
	bool registerTraits(const ZmbTrait &traits);
	/** Append one Booliewood completion snapshot while history capacity remains. */
	bool recordCompletedZoombini(const ZoombiniState &zoombini);
	/** Return whether the profile has accepted its 625th Zoombini registration. */
	bool hasReachedZoombiniRegistrationLimit() const { return _traitRegistrations._totalCount == kZoombiniCombinationCount; }
	/** Return whether accumulated progress has unlocked relaxed party trait limits. */
	bool hasRelaxedPackTraitLimits() const { return 600 <= _traitRegistrations._twiceRegisteredCombinationCount; }

	/** Return whether @p pageId has any recorded visit. */
	bool isPageVisited(int pageId) const {
		if (pageId < 0 || 100 <= pageId)
			return false;
		return _pageLevel[pageId] != 0;
	}

	/** Return the recorded count for one page and visit kind, or zero for invalid input. */
	byte getPageVisitCount(int pageId, int visitKind = 1) const {
		if (visitKind < 1 || 3 < visitKind || pageId < 0 || 24 < pageId)
			return 0;
		return _pageVisitCounts[5 * pageId + visitKind];
	}

	/** Return whether @p pageId has a visit recorded for @p visitKind. */
	bool hasPageVisit(int pageId, int visitKind = 1) const {
		if (visitKind < 1 || 3 < visitKind || pageId < 0 || 24 < pageId)
			return false;
		return getPageVisitCount(pageId, visitKind) != 0;
	}

	/** Increment a page's selected visit counter, saturating at 250. */
	void registerPageVisit(int pageId, int visitKind = 1);

	/** Return whether the first rescue movie has already played. */
	bool hasPlayedRescue1Movie() const { return _rescue1MoviePlayed != 0; }
	/** Record that the first rescue movie has played. */
	void markRescue1MoviePlayed() { _rescue1MoviePlayed = 1; }
	/** Return whether the second rescue movie has already played. */
	bool hasPlayedRescue2Movie() const { return _rescue2MoviePlayed != 0; }
	/** Record that the second rescue movie has played. */
	void markRescue2MoviePlayed() { _rescue2MoviePlayed = 1; }

	/** Return the active level in the inclusive range 1 through 4, defaulting to 1. */
	int getLevel() const {
		if (1 <= _level && _level <= 4)
			return _level;
		return 1;
	}

	/** Player-visible profile name stored in the save file. */
	Common::String _playerName;
	/** Runtime level selected for the active gameplay page. */
	int _level;
	/** Most recently initialized gameplay page. */
	int _currentGameplayPageId;
	/** Serialized compatibility flag with no other known Z2-v1.1KR consumer. */
	byte _legacyStateFlag;
	/** Whether the saved party has reached Rescue Site I. */
	byte _hasReachedRescue1;
	/** Whether the saved party has reached Rescue Site II. */
	byte _hasReachedRescue2;
	/** Whether the saved party has reached Booliewood. */
	byte _hasReachedBooliewood;
	/** Rescue Site I sparse storage board, including its unused sentinel row. */
	BoardRecord *_rescue1Board[kBoardSize] = {};
	/** Rescue Site II sparse storage board, including its unused sentinel row. */
	BoardRecord *_rescue2Board[kBoardSize] = {};
	/** Saved level for each page; a nonzero value also marks the page visited. */
	int32 _pageLevel[100];
	/** Saturating visit counters indexed by `5 * pageId + visitKind`. */
	byte _pageVisitCounts[500];
	/** Serialized compatibility array with no other known Z2-v1.1KR consumer. */
	int32 _legacyData[100];
	/** Total rescued Boolies used for Booliewood development and the 400-point finale. */
	int32 _rescuedBoolieCount;
	/** Cumulative active-party arrivals at Rescue Site I. */
	int32 _rescue1ArrivalCount;
	/** Serialized compatibility statistic with no other known Z2-v1.1KR consumer. */
	int32 _legacyStatistic;
	/** Number of entries used in @ref GameState::_completedTraitHashes. */
	int32 _completedZoombiniCount;
	/** Packed trait snapshots for Zoombinis that reached Booliewood. */
	int32 _completedTraitHashes[kCompletedTraitHashCount];
	/** First-rescue movie completion flag. */
	byte _rescue1MoviePlayed;
	/** Second-rescue movie completion flag. */
	byte _rescue2MoviePlayed;
	/** Trait-combination usage and aggregate registration totals. */
	TraitRegistrationState _traitRegistrations;
	/** Zoombini roster stored in this profile. */
	Common::Array<ZoombiniState *> _savedRoster;

private:
	/** Disallow copying pointers stored in this profile. */
	GameState(const GameState &) = delete;
	/** Disallow assigning pointers stored in this profile. */
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
	/** Count occupied cells in one sparse storage board, excluding its sentinel row. */
	static int countBoardEntries(BoardRecord *const *board);
	/** Return the counter-table index for a validated trait combination. */
	static int getTraitCombinationTableIndex(const ZmbTrait &traits);
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
	/** Return every listed profile with counts parsed directly from its independent .mk file. */
	Common::Array<Zoombini2ProfileSummary> listProfileSummaries() const;
	/** Serialize @p state to @p profileName and, when supplied, its active party roster. */
	bool saveProfile(const Common::String &profileName, const GameState &state, const Common::Array<ZoombiniState *> *globalRoster = nullptr) const;
	/** Load @p profileName transactionally into @p state. */
	bool loadProfile(const Common::String &profileName, GameState &state) const;
	/** Validate an external Z2 stream and store it under @p profileName. */
	bool importProfile(const Common::String &profileName, Common::SeekableReadStream *src, bool overwrite) const;
	/** Validate and copy @p profileName's original-format bytes to @p dest. */
	bool exportProfile(const Common::String &profileName, Common::WriteStream *dest) const;
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
