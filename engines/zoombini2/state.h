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

#ifndef ZOOMBINI2_STATE_H
#define ZOOMBINI2_STATE_H

#include "common/array.h"
#include "common/language.h"
#include "common/noncopyable.h"
#include "common/scummsys.h"
#include "common/str-array.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/ustr.h"
#include "common/util.h"

namespace Common {
class SaveFileManager;
} // namespace Common

namespace Zoombini2 {

class ZoombiniRunner;
class Random;

/** Numeric page identifiers accepted by the engine dispatcher. */
enum PageId : int {
	kPageMenuLoad = -4,         ///< Saved-adventure sign-in flow.
	kPageMenuPractice = -3,     ///< Practice-map sign-in flow.
	kPageMenuOptions = -2,      ///< Sign-in screen.
	kPageNone = -1,             ///< No dispatched page.
	kPageZombiniville = 0,      ///< Zoombiniville party-assembly shelter.
	kPageCrazyTurtle = 1,       ///< Turtle Hurdle puzzle.
	kPageWaterslide = 2,        ///< Pipes of Paloo puzzle.
	kPageAquacube = 3,          ///< Aqua Cube puzzle.
	kPageRescue1 = 4,           ///< First rescue-site shelter.
	kPageMysticMarsh = 5,       ///< Bubble Bumpers puzzle.
	kPageMagicWall = 6,         ///< Beetle Bug Alley puzzle.
	kPageWallOfFleens = 7,      ///< Magic Mirrors puzzle.
	kPageChezNorf = 8,          ///< Chez Norf puzzle.
	kPageRescue2 = 9,           ///< Second rescue-site shelter.
	kPageSnowboard = 10,        ///< Snowboard Gulch puzzle.
	kPageBoolies = 11,          ///< Boolie Boggle puzzle.
	kPageBooliewood = 12,       ///< Booliewood arrival shelter.
	kPageCredits = 16,          ///< Credits transition.
	kPageLogoTLC = 17,          ///< Splash video of The Learning Company
	kPageCutsceneFirst = 18,    ///< First story video.
	kPageCutsceneSecond = 20,   ///< Second story video.
	kPageCutsceneThird = 21,    ///< Third story video.
	kPageMapTrans = 22,         ///< Route-map travel transition
	kPageFinal = 23,            ///< Booliewood final-celebration shelter
	kPageTitleScreen = 24,      ///< Title screen
	kPageLogoPolygon = 25,      ///< Splash video of Polygon Studio
	kPageMapScreen = 30,        ///< ScummVM map-screen dispatcher alias
	kPageMenuAlt = 40,          ///< Alternate sign-in route
	kPageLogoArisuMedia = 1972, ///< (v1.1KR only) Splash video of ArisuMedia
};

/** Stored Zoombini traits with an unused slot zero followed by the four visible values. */
struct ZmbTrait {
	/** Number of visible traits in the tuple. */
	static constexpr int kTraitKindCount = 4;
	/** Number of selectable values for each visible trait. */
	static constexpr int kTraitValueCount = 5;

	/** Identity and tuple index of one visible trait. */
	enum class TraitKind : byte {
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
	byte getValue(TraitKind index) const;
	/** Return whether every trait is in the inclusive range 1 through 5. */
	bool hasValidValues() const;
	/** Return the packed identifier derived from all four traits. */
	uint16 calculateHash() const;
	/** Decode a packed identifier into feet, nose, hair, eyes storage order. */
	static ZmbTrait fromHash(uint16 traitHash);
	/** Return the ScummVM-internal diagnostic name for one visible trait. */
	static const char *debugTraitName(TraitKind index) {
		switch (index) {
		case TraitKind::kFeet00:
			return "feet";
		case TraitKind::kNose01:
			return "nose";
		case TraitKind::kHair02:
			return "hair";
		case TraitKind::kEyes03:
			return "eyes";
		default:
			return "?";
		}
	}
	/** Return the ScummVM-internal diagnostic name for one visible trait value. */
	static const char *debugTraitValueName(TraitKind index, int value) {
		// Rows follow the Hair, Eyes, Nose, Feet display order used by toStr().
		static constexpr const char *kZoombiniTraitNames[kTraitKindCount][kTraitValueCount] = {
			{"Braids", "LongWavy", "Cap", "Ponytail", "Spikey"},
			{"NormalEyed", "Cyclops", "SleepyEyed", "Glasses", "Sunglasses"},
			{"Orange", "White", "Green", "Blue", "Pink"},
			{"Sneakers", "RollerSkates", "Spring", "Wheels", "Propeller"},
		};
		if (value < 1 || kTraitValueCount < value)
			return "?";
		switch (index) {
		case TraitKind::kHair02:
			return kZoombiniTraitNames[0][value - 1];
		case TraitKind::kEyes03:
			return kZoombiniTraitNames[1][value - 1];
		case TraitKind::kNose01:
			return kZoombiniTraitNames[2][value - 1];
		case TraitKind::kFeet00:
			return kZoombiniTraitNames[3][value - 1];
		default:
			break;
		}
		return "?";
	}
	/** Return ScummVM-internal trait names in the Z1 diagnostic output pattern. */
	Common::String toStr() const {
		return Common::String::format("%s, %s, %s, %s",
									  debugTraitValueName(TraitKind::kHair02, _hair),
									  debugTraitValueName(TraitKind::kEyes03, _eyes),
									  debugTraitValueName(TraitKind::kNose01, _nose),
									  debugTraitValueName(TraitKind::kFeet00, _feet));
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

/** Population counts displayed for one saved game. */
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

/** Name, validation state, and population counts for one independent .mk savefile. */
struct Zoombini2SavefileSummary {
	Zoombini2SavefileSummary() = default;

	/** Savefile name derived from the target-scoped .mk filename. */
	Common::String _savefileName;
	/** Whether the complete .mk stream could be parsed. */
	bool _stateValid = false;
	/** Counts parsed from the independent .mk stream when @ref Zoombini2SavefileSummary::_stateValid is true. */
	Zoombini2PopulationSummary _population;
};

/** Editing-session audio levels held by the volume panel's sliders. */
struct VolumeSettings {
	/** Largest accepted volume percentage. */
	static constexpr int kMaxVolumePercent = 100;

	/** Clamp and assign the current music level. */
	void setMusic(int value) { _music = CLIP(value, 0, kMaxVolumePercent); }
	/** Clamp and assign the current sound-effect level. */
	void setSfx(int value) { _sfx = CLIP(value, 0, kMaxVolumePercent); }
	/** Clamp and assign the current speech level. */
	void setSpeech(int value) { _speech = CLIP(value, 0, kMaxVolumePercent); }
	/** Return the current music level percentage. */
	int getMusic() const { return _music; }
	/** Return the current sound-effect level percentage. */
	int getSfx() const { return _sfx; }
	/** Return the current speech level percentage. */
	int getSpeech() const { return _speech; }
	/** Return the music level captured when this edit began. */
	int getInitialMusic() const { return _initialMusic; }
	/** Return the sound-effect level captured when this edit began. */
	int getInitialSfx() const { return _initialSfx; }
	/** Return the speech level captured when this edit began. */
	int getInitialSpeech() const { return _initialSpeech; }
	/** Clamp and assign the current levels, then capture them as the cancellation baseline. */
	void setInitialVolumes(int music, int sfx, int speech) {
		setMusic(music);
		setSfx(sfx);
		setSpeech(speech);
		_initialMusic = _music;
		_initialSfx = _sfx;
		_initialSpeech = _speech;
	}

private:
	/** Current music level percentage. */
	int _music = kMaxVolumePercent;
	/** Current sound-effect level percentage. */
	int _sfx = kMaxVolumePercent;
	/** Current speech level percentage. */
	int _speech = kMaxVolumePercent;
	/** Music level restored when the editing session is cancelled. */
	int _initialMusic = kMaxVolumePercent;
	/** Sound-effect level restored when the editing session is cancelled. */
	int _initialSfx = kMaxVolumePercent;
	/** Speech level restored when the editing session is cancelled. */
	int _initialSpeech = kMaxVolumePercent;
};

/** Twenty-byte representation of one Zoombini stored in a sparse storage cell. */
struct StorageRecord {
	/** Bytes reserved for a fixed-size Zoombini name in storage and saved games. */
	static constexpr int kNameSize = 15;

	/** Initialize the record to an empty name and unset stored traits. */
	StorageRecord() = default;
	/** Copy the persistent fields from @p zoombini into this record. */
	void store(const ZoombiniRunner &zoombini);
	/** Return the complete fixed-size name bytes retained in this record. */
	const char (&getNameBytes() const)[kNameSize] { return _name; }
	/** Return the complete trait tuple stored in this record. */
	ZmbTrait getTraits() const;
	/** Restore a newly allocated active Zoombini from this record. */
	ZoombiniRunner *restore() const;

private:
	friend class GameState;

	/** Fixed-size character name copied from the active Zoombini. */
	char _name[kNameSize] = {};
	/** Unused slot zero followed by Feet, Nose, Hair, and Eyes. */
	ZmbTrait _traits;
};

/** Per-combination use-count table retained by the game state. */
struct TraitComboTable {
	/** Number of distinct Zoombinis represented by the four visible traits. */
	static constexpr int kZoombiniCombinationCount = 625;

	/** Return the table index for a validated trait combination, or -1. */
	static int getComboIndex(const ZmbTrait &traits);
	/** Return this table's count for @p traits, or zero for invalid values. */
	byte getComboCount(const ZmbTrait &traits) const;
	/** Return whether one more copy of @p traits may be registered in this table. */
	bool canRegisterCombo(const ZmbTrait &traits) const;
	/** Register one copy of @p traits in this table if its per-combination limit has not been reached. */
	bool registerCombo(const ZmbTrait &traits);
	/** Remove one prior registration of @p traits from this table. */
	bool unregisterCombo(const ZmbTrait &traits);

private:
	friend class GameState;

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
 * Tracks one game's progress, local roster, sparse storage, and trait-combo table.
 *
 * Loads are transactional: a complete temporary state is validated before it
 * replaces the active instance. The class also owns every pointer stored in
 * @ref GameState::_rescue1Storage, @ref GameState::_rescue2Storage, and
 * @ref GameState::_savedRoster.
 */
class GameState : public Common::NonCopyable {
public:
	/** Save-file version accepted by @ref GameState. */
	static constexpr int kSaveFileMagic = 262;
	/** Number of serialized rows in each sparse storage area. */
	static constexpr int kStorageRows = 125;
	/** Number of serialized columns in each sparse storage area. */
	static constexpr int kStorageCols = 5;
	/** Storage allocation size, including one unused sentinel row. */
	static constexpr int kStorageSize = 630;
	/** Number of completed-Zoombini trait hashes retained in one saved game. */
	static constexpr int kCompletedTraitHashCount = 210;

	/** Generate a short name for a newly created Zoombini. */
	static Common::String generateZoombiniName(Random &random);
	/** Construct a fresh game state. */
	GameState();
	/** Release all storage records and local-roster entries. */
	~GameState();

	/** Reset all serialized and runtime game fields to their initial values. */
	void init();

	/** Load and validate a complete game state from @p stream. */
	bool load(Common::SeekableReadStream *stream);
	/** Serialize this game state and the eligible active party to @p stream. */
	bool save(Common::WriteStream *stream) const;
	/** Delete and clear the live party retained by this game state. */
	void clearActiveZoombinis();
	/** Restore the saved party as live runners. */
	void restoreSavedZoombinis();
	/** Retain the live party as saved roster entries on return to the map. */
	void stashActiveZoombinis();
	/** Retain puzzle leavers in the saved roster or @p storage, keep successful route members active, and record an eligible perfect clear. */
	void finishPuzzleRoster(PageId pageId, StorageRecord **storage, bool advancing, bool savedGame, bool perfectClearEligible);
	/** Delete every record in @p storage and clear its cells. */
	static void clearStorage(StorageRecord **storage);
	/** Store @p zoombini in the first available cell of @p storage. */
	static bool storeInStorage(StorageRecord **storage, ZoombiniRunner &zoombini);
	/** Restore up to @p count Zoombinis from @p storage into @p roster. */
	static void refillFromStorage(StorageRecord **storage, Common::Array<ZoombiniRunner *> &roster, uint count);
	/** Return the storage row used to initialize the shelter scroll position. */
	static int findStorageScrollRow(StorageRecord *const *storage);
	/** Return the four shelter-stage counts and serialized active-party count. */
	Zoombini2PopulationSummary getPopulationSummary() const;
	/** Return the savefile name associated with this game state. */
	const Common::String &getPlayerName() const { return _playerName; }
	/** Associate this state with @p playerName. */
	void setPlayerName(const Common::String &playerName) { _playerName = playerName; }
	/** Append one Booliewood completion snapshot while history capacity remains. */
	bool recordCompletedZoombini(const ZoombiniRunner &zoombini);
	/** Apply one completed Booliewood trip to this game state and its active party. */
	void recordBooliesCompletion();
	/** Return whether this game state has accepted its 625th Zoombini registration. */
	bool hasReachedZoombiniRegistrationLimit() const { return _traitComboTable._totalCount == TraitComboTable::kZoombiniCombinationCount; }
	/** Return whether accumulated progress has unlocked relaxed party trait limits. */
	bool hasRelaxedPackTraitLimits() const { return 600 <= _traitComboTable._twiceRegisteredCombinationCount; }

	/** Return whether @p pageId has any recorded visit. */
	bool isPageVisited(PageId pageId) const {
		const int pageIndex = static_cast<int>(pageId);
		if (pageIndex < 0 || 100 <= pageIndex)
			return false;
		return _pageLevel[pageIndex] != 0;
	}

	/** Return the recorded count for one page and visit kind, or zero for invalid input. */
	byte getPageVisitCount(PageId pageId, int visitKind = 1) const {
		const int pageIndex = static_cast<int>(pageId);
		if (visitKind < 1 || 3 < visitKind || pageIndex < 0 || 24 < pageIndex)
			return 0;
		return _pageVisitCounts[5 * pageIndex + visitKind];
	}

	/** Return whether @p pageId has a visit recorded for @p visitKind. */
	bool hasPageVisit(PageId pageId, int visitKind = 1) const {
		const int pageIndex = static_cast<int>(pageId);
		if (visitKind < 1 || 3 < visitKind || pageIndex < 0 || 24 < pageIndex)
			return false;
		return getPageVisitCount(pageId, visitKind) != 0;
	}

	/** Increment a page's selected visit counter, saturating at 250. */
	void registerPageVisit(PageId pageId, int visitKind = 1);
	/** Return the serialized level for @p pageId, or zero when the page ID is outside the state table. */
	int getPageLevel(PageId pageId) const;
	/** Select @p pageId's serialized level for the next saved-adventure puzzle, initializing an unvisited page to level one. */
	int activatePageLevel(PageId pageId);

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

private:
	/** Player name serialized in the savefile. */
	Common::String _playerName;

public:
	/** Runtime level selected for the active gameplay page. */
	int _level;
	/** Most recently initialized gameplay page. */
	PageId _currentGameplayPageId = kPageZombiniville;
	/** Serialized compatibility flag with no other known Z2-v1.1KR consumer. */
	byte _legacyStateFlag;
	/** Whether the saved party has reached Rescue Site I. */
	byte _hasReachedRescue1;
	/** Whether the saved party has reached Rescue Site II. */
	byte _hasReachedRescue2;
	/** Whether the saved party has reached Booliewood. */
	byte _hasReachedBooliewood;
	/** Rescue Site I sparse storage, including its unused sentinel row. */
	StorageRecord *_rescue1Storage[kStorageSize] = {};
	/** Rescue Site II sparse storage, including its unused sentinel row. */
	StorageRecord *_rescue2Storage[kStorageSize] = {};
	/** Saved level for each page; a nonzero value also marks the page visited. */
	int32 _pageLevel[100];
	/** Saturating visit counters indexed by `5 * pageId + visitKind`. */
	byte _pageVisitCounts[500];
	/** Per-page perfect-clear counts toward the next difficulty increase; each count resets after three. */
	int32 _perfectClearCount[100];
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

private:
	/** First-rescue movie completion flag. */
	byte _rescue1MoviePlayed;
	/** Second-rescue movie completion flag. */
	byte _rescue2MoviePlayed;

public:
	/** Trait-combination use-count table and aggregate totals. */
	TraitComboTable _traitComboTable;
	/** Zoombini roster retained by this game state. */
	Common::Array<ZoombiniRunner *> _savedRoster;
	/** Live party for the current page, separate from the serialized saved roster. */
	Common::Array<ZoombiniRunner *> _activeZoombinis;

private:
	/** Disallow copying because this game state retains runner pointers. */
	GameState(const GameState &) = delete;
	/** Disallow assigning because this game state retains runner pointers. */
	GameState &operator=(const GameState &) = delete;

	/** Delete all retained storage and roster entries. */
	void clearOwnedData();
	/** Copy runner identity into another roster and release the source entries. */
	static void transferRoster(Common::Array<ZoombiniRunner *> &src, Common::Array<ZoombiniRunner *> &dest);
	/** Copy the persistent identity of one live runner into a new roster entry. */
	static ZoombiniRunner *cloneRosterMember(const ZoombiniRunner &src);
	/** Exchange all owned and scalar state with @p other. */
	void swapState(GameState &other);
	/** Parse the complete state body from @p stream. */
	bool readState(Common::SeekableReadStream *stream);
	/** Return whether @p bytes can be read from the stream's current position. */
	static bool canRead(Common::SeekableReadStream *stream, uint64 bytes);
	/** Parse one sparse storage area from @p stream. */
	static bool readStorage(Common::SeekableReadStream *stream, StorageRecord **storage);
	/** Serialize one sparse storage area and return its occupied-cell count. */
	static int writeStorage(Common::WriteStream *stream, StorageRecord *const *storage);
	/** Count occupied storage cells, excluding the sentinel row. */
	static int countStorageEntries(StorageRecord *const *storage);
	/** Exchange two fixed-size arrays element by element. */
	template<typename T, uint N>
	static void swapArray(T (&a)[N], T (&b)[N]) {
		for (uint i = 0; i < N; i++)
			SWAP(a[i], b[i]);
	}
};

/**
 * Target-scoped storage for independent `.mk` savefiles.
 *
 * The manager validates single-byte savefile names, serializes @ref GameState,
 * and verifies newly written bytes before reporting success.
 */
class Zoombini2SavegameManager {
public:
	/** Maximum number of characters accepted in a savefile name. */
	static constexpr int kMaximumSavefileNameLength = 16;

	/** Bind savefile operations to @p saveFileManager and @p target with @p language's name encoding. */
	Zoombini2SavegameManager(Common::SaveFileManager *saveFileManager, const Common::String &target, Common::Language language);

	/** Return safe stored savefile names in case-insensitive sort order, including names unavailable through game input. */
	Common::StringArray listSavefiles() const;
	/** Return whether the listed local savefile for @p savefileName is not writable. */
	bool isSavefileReadOnly(const Common::String &savefileName) const;
	/** Return every listed savefile with counts parsed directly from its independent .mk file. */
	Common::Array<Zoombini2SavefileSummary> listSavefileSummaries() const;
	/** Serialize @p state and its eligible active party under @p savefileName. */
	bool writeSavefile(const Common::String &savefileName, const GameState &state) const;
	/** Load @p savefileName transactionally into @p state. */
	bool loadSavefile(const Common::String &savefileName, GameState &state) const;
	/** Validate an external Z2 stream and store it under @p savefileName. */
	bool importSavefile(const Common::String &savefileName, Common::SeekableReadStream *src, bool overwrite) const;
	/** Validate and copy @p savefileName's original-format bytes to @p dest. */
	bool exportSavefile(const Common::String &savefileName, Common::WriteStream *dest) const;
	/** Delete the saved game named @p savefileName. */
	bool deleteSavefile(const Common::String &savefileName) const;
	/** Rename a savefile without overwriting another savefile. */
	bool renameSavefile(const Common::String &oldSavefileName, const Common::String &newSavefileName) const;
	/** Create a distinct copy of one savefile without overwriting another savefile. */
	bool duplicateSavefile(const Common::String &srcSavefileName, const Common::String &newSavefileName) const;
	/** Return whether a safe stored savefile has a target-scoped save file. */
	bool savefileExists(const Common::String &savefileName) const;

	/** Validate a new name against @p language's single-byte typing set. */
	static bool isValidSavefileName(const Common::String &savefileName, Common::Language language);
	/** Convert a GUI name to the language's single-byte form without losing characters. */
	bool encodeSavefileName(const Common::U32String &displayName, Common::String &savefileName) const;
	/** Convert a stored single-byte name for display in the ScummVM GUI. */
	Common::U32String decodeSavefileName(const Common::String &savefileName) const;

private:
	/** Check length and path safety for a previously stored name without imposing the typing whitelist. */
	static bool isSafeStoredSavefileName(const Common::String &savefileName);
	/** Build the target-scoped filename for @p savefileName. */
	Common::String makeSaveFileName(const Common::String &savefileName) const;
	/** Insert @p savefileName into @p savefiles unless it is already present. */
	static void addSavefileSorted(Common::StringArray &savefiles, const Common::String &savefileName);
	/** Verify that a completed save contains exactly @p size bytes from @p data. */
	bool verifySaveData(const Common::String &saveFileName, const byte *data, uint32 size) const;

	/** Backend that owns target save streams. */
	Common::SaveFileManager *_saveFileManager;
	/** ScummVM target identifier used as the save-file namespace. */
	Common::String _target;
	/** Language used to encode, decode, and validate savefile names. */
	Common::Language _language;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_STATE_H
