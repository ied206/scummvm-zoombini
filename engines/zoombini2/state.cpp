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

#include <string.h>

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-enc.h"

#include "zoombini2/scripts.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

Common::String GameState::generateZoombiniName(Random &random) {
	static constexpr const char *kVowelPairs[30] = {
		"a ",
		"a ",
		"a ",
		"e ",
		"e ",
		"e ",
		"e ",
		"i ",
		"i ",
		"i ",
		"o ",
		"o ",
		"o ",
		"u ",
		"u ",
		"y ",
		"ee",
		"oo",
		"yo",
		"ya",
		"ye",
		"ei",
		"ie",
		"ai",
		"ia",
		"au",
		"ua",
		"uo",
		"ou",
		"ae",
	};
	static constexpr char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwx";
	static constexpr char kEndings[] = "aeiou";
	static constexpr const char *kConsonantPairs[39] = {
		"bl",
		"br",
		"ch",
		"cl",
		"cr",
		"dr",
		"dw",
		"fl",
		"fr",
		"gh",
		"gl",
		"gr",
		"kl",
		"kn",
		"kr",
		"kw",
		"ld",
		"mp",
		"nd",
		"nh",
		"nn",
		"ph",
		"pl",
		"pr",
		"qu",
		"qu",
		"rh",
		"rn",
		"sc",
		"sl",
		"sm",
		"sn",
		"sp",
		"sr",
		"st",
		"sw",
		"th",
		"tr",
		"tw",
	};

	char name[8] = {};
	const int targetLength = random.getRandomNumber(1) + 4;
	bool useVowelPair = random.getRandomNumber(98) + 1 < 40;
	int length = 0;
	while (length < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[random.getRandomNumber(29)];
			if (pair[1] != ' ') {
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
			} else {
				name[length] = pair[0];
				name[length] = pair[0];
				length += 1;
			}
		} else {
			useVowelPair = true;
			if (1 < length || random.getRandomNumber(98) + 1 <= 33) {
				const char *pair = kConsonantPairs[random.getRandomNumber(38)];
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
				usedConsonantPair = true;
			} else {
				name[length] = kSingleConsonants[random.getRandomNumber(30)];
				length += 1;
			}
		}
		if (usedConsonantPair && targetLength <= length)
			name[length - 1] = kEndings[random.getRandomNumber(4)];
		if (length == 2 && name[0] == name[1])
			length = 1;
	}
	return Common::String(name);
}

byte ZmbTrait::getValue(TraitKind index) const {
	switch (index) {
	case TraitKind::kFeet00:
		return _feet;
	case TraitKind::kNose01:
		return _nose;
	case TraitKind::kHair02:
		return _hair;
	case TraitKind::kEyes03:
		return _eyes;
	default:
		return 0;
	}
}

bool ZmbTrait::hasValidValues() const {
	return 1 <= _feet && _feet <= kTraitValueCount && 1 <= _nose && _nose <= kTraitValueCount &&
		   1 <= _hair && _hair <= kTraitValueCount && 1 <= _eyes && _eyes <= kTraitValueCount;
}

uint16 ZmbTrait::calculateHash() const {
	return _feet + 8 * (_nose + 8 * (_eyes + 8 * _hair));
}

ZmbTrait ZmbTrait::fromHash(uint16 traitHash) {
	const byte feet = static_cast<byte>(traitHash & 7);
	const byte nose = static_cast<byte>((traitHash >> 3) & 7);
	const byte eyes = static_cast<byte>((traitHash >> 6) & 7);
	const byte hair = static_cast<byte>((traitHash >> 9) & 7);
	return ZmbTrait(feet, nose, hair, eyes);
}

void StorageRecord::store(const ZoombiniRunner &zoombini) {
	memcpy(_name, zoombini._name, sizeof(_name));
	_traits = zoombini._traits;
}

ZmbTrait StorageRecord::getTraits() const {
	return _traits;
}

ZoombiniRunner *StorageRecord::restore() const {
	ZoombiniRunner *zoombini = new ZoombiniRunner();
	memcpy(zoombini->_name, _name, sizeof(zoombini->_name));
	zoombini->setTraits(getTraits());
	zoombini->setInputEnabled(true);
	zoombini->setAnimationCell(33);
	return zoombini;
}

void GameState::clearStorage(StorageRecord **storage) {
	for (int i = 0; i < kStorageRows * kStorageCols; i++) {
		delete storage[i];
		storage[i] = nullptr;
	}
}

bool GameState::storeInStorage(StorageRecord **storage, ZoombiniRunner &zoombini) {
	int startRow = 62;
	for (int i = 0; i < kStorageRows * kStorageCols; i++) {
		if (storage[i]) {
			startRow = i / kStorageCols;
			break;
		}
	}
	for (int direction = 1; -1 <= direction; direction -= 2) {
		for (int row = startRow; 0 <= row && row < kStorageRows; row += direction) {
			for (int col = 0; col < kStorageCols; col++) {
				StorageRecord *&cell = storage[row * kStorageCols + col];
				if (!cell) {
					cell = new StorageRecord();
					cell->store(zoombini);
					zoombini.setCanAdvanceFromPage(false);
					return true;
				}
			}
		}
	}
	return false;
}

void GameState::refillFromStorage(StorageRecord **storage, Common::Array<ZoombiniRunner *> &roster, uint count) {
	for (int col = 0; col < kStorageCols && roster.size() < count; col++) {
		for (int row = 0; row < kStorageRows && roster.size() < count; row++) {
			StorageRecord *&cell = storage[row * kStorageCols + col];
			if (cell) {
				roster.push_back(cell->restore());
				delete cell;
				cell = nullptr;
			}
		}
	}
}

int GameState::findStorageScrollRow(StorageRecord *const *storage) {
	for (int direction = 1; -1 <= direction; direction -= 2) {
		for (int col = 0; col < kStorageCols; col++) {
			for (int row = 62; 0 <= row && row < kStorageRows; row += direction) {
				if (storage[row * kStorageCols + col])
					return kStorageRows < row + 6 ? 121 : row;
			}
		}
	}
	return 62;
}

int TraitComboTable::getComboIndex(const ZmbTrait &traits) {
	if (!traits.hasValidValues())
		return -1;
	const int eyesAndHair = (traits._eyes - 1) + ZmbTrait::kTraitValueCount * (traits._hair - 1);
	const int noseEyesAndHair = (traits._nose - 1) + ZmbTrait::kTraitValueCount * eyesAndHair;
	return (traits._feet - 1) + ZmbTrait::kTraitValueCount * noseEyesAndHair;
}

byte TraitComboTable::getComboCount(const ZmbTrait &traits) const {
	const int tableIndex = getComboIndex(traits);
	if (tableIndex < 0)
		return 0;
	return _combinationUseCounts[tableIndex];
}

bool TraitComboTable::canRegisterCombo(const ZmbTrait &traits) const {
	return traits.hasValidValues() && getComboCount(traits) < 2;
}

bool TraitComboTable::registerCombo(const ZmbTrait &traits) {
	const int tableIndex = getComboIndex(traits);
	if (tableIndex < 0)
		return false;

	const byte previousCount = _combinationUseCounts[tableIndex];
	if (2 <= previousCount)
		return false;
	if (previousCount == 0)
		_uniqueCombinationCount += 1;
	const byte count = previousCount + 1;
	_combinationUseCounts[tableIndex] = count;
	if (count == 2)
		_twiceRegisteredCombinationCount += 1;
	_totalCount += 1;
	return true;
}

bool TraitComboTable::unregisterCombo(const ZmbTrait &traits) {
	const int tableIndex = getComboIndex(traits);
	if (tableIndex < 0)
		return false;

	const byte previousCount = _combinationUseCounts[tableIndex];
	if (previousCount == 0 || _totalCount <= 0)
		return false;
	if (previousCount == 2) {
		if (_twiceRegisteredCombinationCount <= 0)
			return false;
		_twiceRegisteredCombinationCount -= 1;
	}
	if (previousCount == 1) {
		if (_uniqueCombinationCount <= 0)
			return false;
		_uniqueCombinationCount -= 1;
	}
	_combinationUseCounts[tableIndex] = previousCount - 1;
	_totalCount -= 1;
	return true;
}

bool GameState::recordCompletedZoombini(const ZoombiniRunner &zoombini) {
	if (_completedZoombiniCount < 0)
		return false;
	if (_completedZoombiniCount < kCompletedTraitHashCount) {
		_completedTraitHashes[_completedZoombiniCount] = zoombini._traitHash;
		_completedZoombiniCount += 1;
		return true;
	}
	return false;
}

void GameState::recordBooliesCompletion() {
	if (_activeZoombinis.empty() || !_activeZoombinis[0])
		return;

	const int rescuedBooliesPerZoombini = _activeZoombinis[0]->getRescuedBooliesPerZoombini();
	_rescuedBoolieCount += static_cast<int32>(_activeZoombinis.size()) * rescuedBooliesPerZoombini;
	for (uint i = 0; i < _activeZoombinis.size(); i++) {
		if (_activeZoombinis[i])
			recordCompletedZoombini(*_activeZoombinis[i]);
	}
}

GameState::GameState() {
	init();
}

GameState::~GameState() {
	clearOwnedData();
}

void GameState::clearActiveZoombinis() {
	for (uint i = 0; i < _activeZoombinis.size(); i++)
		delete _activeZoombinis[i];
	_activeZoombinis.clear();
}

void GameState::clearOwnedData() {
	clearActiveZoombinis();
	for (int i = 0; i < kStorageSize; i++) {
		delete _rescue1Storage[i];
		delete _rescue2Storage[i];
		_rescue1Storage[i] = nullptr;
		_rescue2Storage[i] = nullptr;
	}
	for (uint i = 0; i < _savedRoster.size(); i++)
		delete _savedRoster[i];
	_savedRoster.clear();
}

void GameState::init() {
	clearOwnedData();
	_playerName.clear();
	_level = 0;
	_currentGameplayPageId = kPageZombiniville;
	_legacyStateFlag = 0;
	_hasReachedRescue1 = 0;
	_hasReachedRescue2 = 0;
	_hasReachedBooliewood = 0;
	memset(_pageLevel, 0, sizeof(_pageLevel));
	memset(_pageVisitCounts, 0, sizeof(_pageVisitCounts));
	memset(_perfectClearCount, 0, sizeof(_perfectClearCount));
	_rescuedBoolieCount = 0;
	_rescue1ArrivalCount = 0;
	_legacyStatistic = 0;
	_completedZoombiniCount = 0;
	memset(_completedTraitHashes, 0, sizeof(_completedTraitHashes));
	_rescue1MoviePlayed = 0;
	_rescue2MoviePlayed = 0;
	_traitComboTable._totalCount = 1;
	_traitComboTable._uniqueCombinationCount = 1;
	_traitComboTable._twiceRegisteredCombinationCount = 0;
	memset(_traitComboTable._combinationUseCounts, 0, sizeof(_traitComboTable._combinationUseCounts));
	memset(_traitComboTable._unusedTail, 0, sizeof(_traitComboTable._unusedTail));
	// Seed the counter for the initial Feet 1, Nose 4, Hair 5, Eyes 5 combination.
	const int initialCombinationIndex = TraitComboTable::getComboIndex(ZmbTrait(1, 4, 5, 5));
	_traitComboTable._combinationUseCounts[initialCombinationIndex] = 1;
}

void GameState::swapState(GameState &other) {
	SWAP(_playerName, other._playerName);
	SWAP(_level, other._level);
	SWAP(_currentGameplayPageId, other._currentGameplayPageId);
	SWAP(_legacyStateFlag, other._legacyStateFlag);
	SWAP(_hasReachedRescue1, other._hasReachedRescue1);
	SWAP(_hasReachedRescue2, other._hasReachedRescue2);
	SWAP(_hasReachedBooliewood, other._hasReachedBooliewood);
	swapArray(_rescue1Storage, other._rescue1Storage);
	swapArray(_rescue2Storage, other._rescue2Storage);
	swapArray(_pageLevel, other._pageLevel);
	swapArray(_pageVisitCounts, other._pageVisitCounts);
	swapArray(_perfectClearCount, other._perfectClearCount);
	SWAP(_rescuedBoolieCount, other._rescuedBoolieCount);
	SWAP(_rescue1ArrivalCount, other._rescue1ArrivalCount);
	SWAP(_legacyStatistic, other._legacyStatistic);
	SWAP(_completedZoombiniCount, other._completedZoombiniCount);
	swapArray(_completedTraitHashes, other._completedTraitHashes);
	SWAP(_rescue1MoviePlayed, other._rescue1MoviePlayed);
	SWAP(_rescue2MoviePlayed, other._rescue2MoviePlayed);
	SWAP(_traitComboTable._totalCount, other._traitComboTable._totalCount);
	SWAP(_traitComboTable._uniqueCombinationCount, other._traitComboTable._uniqueCombinationCount);
	SWAP(_traitComboTable._twiceRegisteredCombinationCount, other._traitComboTable._twiceRegisteredCombinationCount);
	swapArray(_traitComboTable._combinationUseCounts, other._traitComboTable._combinationUseCounts);
	swapArray(_traitComboTable._unusedTail, other._traitComboTable._unusedTail);
	_savedRoster.swap(other._savedRoster);
	_activeZoombinis.swap(other._activeZoombinis);
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

void GameState::restoreSavedZoombinis() {
	transferRoster(_savedRoster, _activeZoombinis);
}

void GameState::stashActiveZoombinis() {
	transferRoster(_activeZoombinis, _savedRoster);
}

void GameState::finishPuzzleRoster(PageId pageId, StorageRecord **storage, bool advancing, bool savedGame, bool perfectClearEligible) {
	if (!savedGame) {
		clearActiveZoombinis();
		return;
	}

	const uint expectedPartySize = storage ? 8 : 16;
	const int pageIndex = static_cast<int>(pageId);
	if (perfectClearEligible && 0 <= pageIndex && pageIndex < 100 && _activeZoombinis.size() == expectedPartySize) {
		bool allSucceeded = true;
		for (uint i = 0; i < _activeZoombinis.size(); i++) {
			if (!_activeZoombinis[i] || !_activeZoombinis[i]->canAdvanceFromPage()) {
				allSucceeded = false;
				break;
			}
		}
		if (allSucceeded) {
			_perfectClearCount[pageIndex] += 1;
			if (_perfectClearCount[pageIndex] == 3) {
				_perfectClearCount[pageIndex] = 0;
				if (_pageLevel[pageIndex] < 3)
					_pageLevel[pageIndex] += 1;
			}
		}
	}

	for (uint i = 0; i < _activeZoombinis.size();) {
		ZoombiniRunner *zoombini = _activeZoombinis[i];
		if (!advancing || !zoombini->canAdvanceFromPage()) {
			if (storage)
				storeInStorage(storage, *zoombini);
			else
				_savedRoster.push_back(cloneRosterMember(*zoombini));
			delete zoombini;
			_activeZoombinis.remove_at(i);
		} else {
			i += 1;
		}
	}
}

ZoombiniRunner *GameState::cloneRosterMember(const ZoombiniRunner &src) {
	ZoombiniRunner *copy = new ZoombiniRunner();
	copy->_traits = src._traits;
	copy->_traitHash = copy->_traits.calculateHash();
	memcpy(copy->_name, src._name, sizeof(copy->_name));
	return copy;
}

void GameState::transferRoster(Common::Array<ZoombiniRunner *> &src, Common::Array<ZoombiniRunner *> &dest) {
	assert(&src != &dest);
	for (uint i = 0; i < src.size(); i++) {
		const ZoombiniRunner *previous = src[i];
		dest.push_back(cloneRosterMember(*previous));
		delete previous;
	}
	src.clear();
}

bool GameState::readStorage(Common::SeekableReadStream *stream, StorageRecord **storage) {
	if (!canRead(stream, 4))
		return false;
	const int32 count = stream->readSint32LE();
	if (count < 0 || kStorageRows * kStorageCols < count || !canRead(stream, static_cast<uint64>(count) * 28))
		return false;
	for (int32 i = 0; i < count; i++) {
		const int32 row = stream->readSint32LE();
		const int32 col = stream->readSint32LE();
		if (row < 0 || kStorageRows <= row || col < 0 || kStorageCols <= col)
			return false;
		const int index = row * kStorageCols + col;
		if (storage[index])
			return false;
		storage[index] = new StorageRecord();
		StorageRecord &record = *storage[index];
		if (stream->read(record._name, sizeof(record._name)) != sizeof(record._name))
			return false;
		const byte unusedSlot0 = stream->readByte();
		const byte feet = stream->readByte();
		const byte nose = stream->readByte();
		const byte hair = stream->readByte();
		const byte eyes = stream->readByte();
		record._traits = ZmbTrait(feet, nose, hair, eyes);
		record._traits._unusedSlot0 = unusedSlot0;
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

	if (stream->read(_pageVisitCounts, sizeof(_pageVisitCounts)) != sizeof(_pageVisitCounts))
		return false;
	_rescuedBoolieCount = stream->readSint32LE();
	for (int i = 0; i < 100; i++)
		_pageLevel[i] = stream->readSint32LE();
	for (int i = 0; i < 100; i++)
		_perfectClearCount[i] = stream->readSint32LE();
	_hasReachedRescue1 = stream->readByte();
	_hasReachedRescue2 = stream->readByte();
	_hasReachedBooliewood = stream->readByte();
	_legacyStateFlag = stream->readByte();
	_rescue1ArrivalCount = stream->readSint32LE();
	_legacyStatistic = stream->readSint32LE();
	_completedZoombiniCount = stream->readSint32LE();
	for (int i = 0; i < kCompletedTraitHashCount; i++)
		_completedTraitHashes[i] = stream->readSint32LE();
	_rescue1MoviePlayed = stream->readByte();
	_rescue2MoviePlayed = stream->readByte();
	_traitComboTable._totalCount = stream->readSint32LE();
	_traitComboTable._uniqueCombinationCount = stream->readSint32LE();
	_traitComboTable._twiceRegisteredCombinationCount = stream->readSint32LE();
	if (stream->read(_traitComboTable._combinationUseCounts, sizeof(_traitComboTable._combinationUseCounts)) !=
			sizeof(_traitComboTable._combinationUseCounts) ||
		stream->read(_traitComboTable._unusedTail, sizeof(_traitComboTable._unusedTail)) != sizeof(_traitComboTable._unusedTail))
		return false;

	if (!readStorage(stream, _rescue1Storage) || !readStorage(stream, _rescue2Storage) || !canRead(stream, 4))
		return false;
	const int32 count = stream->readSint32LE();
	if (count < 0 || !canRead(stream, static_cast<uint64>(count) * 20))
		return false;
	// The party is stored in two passes, unlike the interleaved sparse storage records.
	for (int32 i = 0; i < count; i++) {
		ZoombiniRunner *zoombini = new ZoombiniRunner();
		_savedRoster.push_back(zoombini);
		const byte unusedSlot0 = stream->readByte();
		const byte feet = stream->readByte();
		const byte nose = stream->readByte();
		const byte hair = stream->readByte();
		const byte eyes = stream->readByte();
		ZmbTrait traits(feet, nose, hair, eyes);
		traits._unusedSlot0 = unusedSlot0;
		zoombini->_traits = traits;
		zoombini->_traitHash = traits.calculateHash();
	}
	for (int32 i = 0; i < count; i++) {
		if (stream->read(_savedRoster[i]->_name, sizeof(_savedRoster[i]->_name)) != sizeof(_savedRoster[i]->_name))
			return false;
	}
	return !stream->err() && !stream->eos();
}

int GameState::writeStorage(Common::WriteStream *stream, StorageRecord *const *storage) {
	int count = 0;
	for (int i = 0; i < kStorageRows * kStorageCols; i++) {
		if (storage[i])
			count += 1;
	}
	stream->writeSint32LE(count);
	for (int i = 0; i < kStorageRows * kStorageCols; i++) {
		if (storage[i]) {
			stream->writeSint32LE(i / kStorageCols);
			stream->writeSint32LE(i % kStorageCols);
			const StorageRecord &record = *storage[i];
			stream->write(record._name, sizeof(record._name));
			stream->writeByte(record._traits._unusedSlot0);
			stream->writeByte(record._traits._feet);
			stream->writeByte(record._traits._nose);
			stream->writeByte(record._traits._hair);
			stream->writeByte(record._traits._eyes);
		}
	}
	return count;
}

int GameState::countStorageEntries(StorageRecord *const *storage) {
	int count = 0;
	for (int i = 0; i < kStorageRows * kStorageCols; i++) {
		if (storage[i])
			count += 1;
	}
	return count;
}

Zoombini2PopulationSummary GameState::getPopulationSummary() const {
	Zoombini2PopulationSummary summary;
	summary._rescue1Count = countStorageEntries(_rescue1Storage);
	summary._rescue2Count = countStorageEntries(_rescue2Storage);
	summary._booliewoodCount = _completedZoombiniCount;
	summary._zombinivilleCount = TraitComboTable::kZoombiniCombinationCount - summary._rescue1Count - summary._rescue2Count - summary._booliewoodCount;
	summary._activePartyCount = static_cast<int>(_savedRoster.size());
	return summary;
}

bool GameState::save(Common::WriteStream *stream) const {
	if (!stream || stream->err())
		return false;
	const uint32 activeCount = kPageZombiniville <= _currentGameplayPageId && _currentGameplayPageId <= kPageAquacube ? _activeZoombinis.size() : 0;
	const uint64 count = static_cast<uint64>(_savedRoster.size()) + activeCount;
	if (static_cast<uint64>(INT32_MAX) < count || static_cast<uint32>(INT32_MAX) <= _playerName.size())
		return false;
	const uint32 nameLength = _playerName.size() + 1;
	const int64 startPosition = stream->pos();
	stream->writeSint32LE(kSaveFileMagic);
	stream->writeUint32LE(nameLength);
	stream->write(_playerName.c_str(), nameLength);
	stream->write(_pageVisitCounts, sizeof(_pageVisitCounts));
	stream->writeSint32LE(_rescuedBoolieCount);
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_pageLevel[i]);
	for (int i = 0; i < 100; i++)
		stream->writeSint32LE(_perfectClearCount[i]);
	stream->writeByte(_hasReachedRescue1);
	stream->writeByte(_hasReachedRescue2);
	stream->writeByte(_hasReachedBooliewood);
	stream->writeByte(_legacyStateFlag);
	stream->writeSint32LE(_rescue1ArrivalCount);
	stream->writeSint32LE(_legacyStatistic);
	stream->writeSint32LE(_completedZoombiniCount);
	for (int i = 0; i < kCompletedTraitHashCount; i++)
		stream->writeSint32LE(_completedTraitHashes[i]);
	stream->writeByte(_rescue1MoviePlayed);
	stream->writeByte(_rescue2MoviePlayed);
	stream->writeSint32LE(_traitComboTable._totalCount);
	stream->writeSint32LE(_traitComboTable._uniqueCombinationCount);
	stream->writeSint32LE(_traitComboTable._twiceRegisteredCombinationCount);
	stream->write(_traitComboTable._combinationUseCounts, sizeof(_traitComboTable._combinationUseCounts));
	stream->write(_traitComboTable._unusedTail, sizeof(_traitComboTable._unusedTail));
	const int rescue1StorageCount = writeStorage(stream, _rescue1Storage);
	const int rescue2StorageCount = writeStorage(stream, _rescue2Storage);
	stream->writeUint32LE(static_cast<uint32>(count));
	for (uint64 i = 0; i < count; i++) {
		const ZoombiniRunner *zoombini;
		if (i < _savedRoster.size())
			zoombini = _savedRoster[i];
		else
			zoombini = _activeZoombinis[i - _savedRoster.size()];
		if (!zoombini)
			return false;
		const ZmbTrait &traits = zoombini->_traits;
		stream->writeByte(traits._unusedSlot0);
		stream->writeByte(traits._feet);
		stream->writeByte(traits._nose);
		stream->writeByte(traits._hair);
		stream->writeByte(traits._eyes);
	}
	for (uint64 i = 0; i < count; i++) {
		const ZoombiniRunner *zoombini;
		if (i < _savedRoster.size())
			zoombini = _savedRoster[i];
		else
			zoombini = _activeZoombinis[i - _savedRoster.size()];
		stream->write(zoombini->_name, sizeof(zoombini->_name));
	}
	const int64 expectedSize = 2822 + static_cast<int64>(nameLength) + (rescue1StorageCount + rescue2StorageCount) * 28 + count * 20;
	return !stream->err() && 0 <= startPosition && stream->pos() - startPosition == expectedSize;
}

void GameState::registerPageVisit(PageId pageId, int visitKind) {
	const int pageIndex = static_cast<int>(pageId);
	if (pageIndex < 0 || 24 < pageIndex || visitKind < 1 || 3 < visitKind)
		return;
	byte &visits = _pageVisitCounts[5 * pageIndex + visitKind];
	if (visits < 250)
		visits += 1;
}

int GameState::getPageLevel(PageId pageId) const {
	const int pageIndex = static_cast<int>(pageId);
	if (pageIndex < 0 || 100 <= pageIndex)
		return 0;
	return _pageLevel[pageIndex];
}

int GameState::activatePageLevel(PageId pageId) {
	const int pageIndex = static_cast<int>(pageId);
	if (pageIndex < 0 || 100 <= pageIndex)
		return 0;

	if (_pageLevel[pageIndex] == 0)
		_pageLevel[pageIndex] = 1;
	_level = _pageLevel[pageIndex];
	return getLevel();
}

Zoombini2SavegameManager::Zoombini2SavegameManager(Common::SaveFileManager *saveFileManager, const Common::String &target, Common::Language language)
	: _saveFileManager(saveFileManager), _target(target.empty() ? "zoombini2" : target), _language(language) {
}

Common::String Zoombini2SavegameManager::makeSaveFileName(const Common::String &savefileName) const {
	Common::String filesystemName = savefileName;
	if (_language == Common::HE_ISR)
		filesystemName = savefileName.decode(Common::kWindows1255).encode(Common::kUtf8);
	else if (_language == Common::SV_SWE)
		filesystemName = savefileName.decode(Common::kWindows1252).encode(Common::kUtf8);
	return Common::String::format("%s-%s.mk", _target.c_str(), filesystemName.c_str());
}

bool Zoombini2SavegameManager::isValidSavefileName(const Common::String &savefileName, Common::Language language) {
	if (savefileName.empty() || kMaximumSavefileNameLength < static_cast<int>(savefileName.size()) || savefileName.firstChar() == ' ')
		return false;

	const bool hebrew = language == Common::HE_ISR;
	bool previousWasSpace = false;
	for (uint i = 0; i < savefileName.size(); i++) {
		const byte character = static_cast<byte>(savefileName[i]);
		const bool isLetter = ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z');
		const bool isDigit = !hebrew && '0' <= character && character <= '9';
		const bool isHebrewLetter = hebrew && 0xE0 <= character && character <= 0xFA;
		const bool isHebrewPunctuation = hebrew && (character == ',' || character == '.' || character == ';');
		const bool isSpace = character == ' ';
		if ((!isLetter && !isDigit && !isHebrewLetter && !isHebrewPunctuation && !isSpace) ||
			(isSpace && previousWasSpace))
			return false;
		previousWasSpace = isSpace;
	}
	return true;
}

bool Zoombini2SavegameManager::isSafeStoredSavefileName(const Common::String &savefileName) {
	static constexpr const char *kInvalidFileNameCharacters = "/\\:*?\"<>|";
	if (savefileName.empty() || kMaximumSavefileNameLength < static_cast<int>(savefileName.size()))
		return false;
	for (uint i = 0; i < savefileName.size(); i++) {
		const byte character = static_cast<byte>(savefileName[i]);
		if (character < 0x20 || character == 0x7F || strchr(kInvalidFileNameCharacters, character) != nullptr)
			return false;
	}
	return true;
}

bool Zoombini2SavegameManager::encodeSavefileName(const Common::U32String &displayName, Common::String &savefileName) const {
	Common::CodePage codePage = Common::kUtf8;
	if (_language == Common::HE_ISR)
		codePage = Common::kWindows1255;
	else if (_language == Common::SV_SWE)
		codePage = Common::kWindows1252;
	savefileName = displayName.encode(codePage);
	return savefileName.decode(codePage) == displayName;
}

Common::U32String Zoombini2SavegameManager::decodeSavefileName(const Common::String &savefileName) const {
	if (_language == Common::HE_ISR)
		return savefileName.decode(Common::kWindows1255);
	if (_language == Common::SV_SWE)
		return savefileName.decode(Common::kWindows1252);
	return savefileName.decode(Common::kUtf8);
}

void Zoombini2SavegameManager::addSavefileSorted(Common::StringArray &savefiles, const Common::String &savefileName) {
	uint index = 0;
	while (index < savefiles.size() && savefiles[index].compareToIgnoreCase(savefileName) < 0)
		index += 1;
	if (index < savefiles.size() && savefiles[index].equalsIgnoreCase(savefileName))
		return;
	savefiles.insert_at(index, savefileName);
}

Common::StringArray Zoombini2SavegameManager::listSavefiles() const {
	Common::StringArray savefiles;
	if (!_saveFileManager)
		return savefiles;

	const Common::String prefix = _target + "-";
	const Common::String suffix = ".mk";
	const Common::StringArray saveFiles = _saveFileManager->listSavefiles(prefix + "*" + suffix);
	for (uint i = 0; i < saveFiles.size(); i++) {
		const Common::String &saveFileName = saveFiles[i];
		if (saveFileName.size() <= prefix.size() + suffix.size())
			continue;
		if (!saveFileName.substr(0, prefix.size()).equalsIgnoreCase(prefix) || !saveFileName.substr(saveFileName.size() - suffix.size()).equalsIgnoreCase(suffix))
			continue;

		const Common::String filesystemName = saveFileName.substr(prefix.size(), saveFileName.size() - prefix.size() - suffix.size());
		Common::String savefileName = filesystemName;
		if (_language == Common::HE_ISR)
			savefileName = filesystemName.decode(Common::kUtf8).encode(Common::kWindows1255);
		else if (_language == Common::SV_SWE)
			savefileName = filesystemName.decode(Common::kUtf8).encode(Common::kWindows1252);
		const bool invalidEncoding = (_language == Common::HE_ISR && savefileName.decode(Common::kWindows1255).encode(Common::kUtf8) != filesystemName) ||
									 (_language == Common::SV_SWE && savefileName.decode(Common::kWindows1252).encode(Common::kUtf8) != filesystemName);
		if (invalidEncoding || !isSafeStoredSavefileName(savefileName)) {
			warning("Zoombini2SavegameManager: Ignoring invalid savefile '%s'", saveFileName.c_str());
			continue;
		}
		addSavefileSorted(savefiles, savefileName);
	}
	return savefiles;
}

bool Zoombini2SavegameManager::isSavefileReadOnly(const Common::String &savefileName) const {
	if (!_saveFileManager || !isSafeStoredSavefileName(savefileName))
		return false;

	const Common::String saveFileName = makeSaveFileName(savefileName);
	if (!_saveFileManager->exists(saveFileName))
		return false;

	const Common::FSNode saveDirectory(ConfMan.getPath("savepath"));
	const Common::FSNode saveFile = saveDirectory.getChild(saveFileName);
	return saveFile.exists() && !saveFile.isWritable();
}

Common::Array<Zoombini2SavefileSummary> Zoombini2SavegameManager::listSavefileSummaries() const {
	const Common::StringArray savefiles = listSavefiles();
	Common::Array<Zoombini2SavefileSummary> summaries;
	for (uint i = 0; i < savefiles.size(); i++) {
		Zoombini2SavefileSummary summary;
		summary._savefileName = savefiles[i];
		GameState state;
		summary._stateValid = loadSavefile(summary._savefileName, state);
		if (summary._stateValid)
			summary._population = state.getPopulationSummary();
		summaries.push_back(summary);
	}
	return summaries;
}

bool Zoombini2SavegameManager::verifySaveData(const Common::String &saveFileName, const byte *data, uint32 size) const {
	Common::InSaveFile *stream = _saveFileManager->openForLoading(saveFileName);
	if (!stream)
		return false;

	bool matches = stream->size() == size;
	byte buffer[4096];
	for (uint32 offset = 0; matches && offset < size;) {
		const uint32 amount = MIN<uint32>(sizeof(buffer), size - offset);
		matches = stream->read(buffer, amount) == amount && memcmp(buffer, data + offset, amount) == 0 && !stream->err();
		offset += amount;
	}
	delete stream;
	return matches;
}

bool Zoombini2SavegameManager::writeSavefile(const Common::String &savefileName, const GameState &state) const {
	if (!_saveFileManager || !isSafeStoredSavefileName(savefileName) ||
		(!isValidSavefileName(savefileName, _language) && !_saveFileManager->exists(makeSaveFileName(savefileName))))
		return false;

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	if (!state.save(&data))
		return false;

	const Common::String saveFileName = makeSaveFileName(savefileName);
	Common::OutSaveFile *stream = _saveFileManager->openForSaving(saveFileName, false);
	if (!stream) {
		warning("Zoombini2SavegameManager: Could not open '%s' for writing", saveFileName.c_str());
		return false;
	}

	bool ok = stream->write(data.getData(), data.size()) == data.size();
	stream->finalize();
	ok = ok && !stream->err();
	delete stream;
	if (ok)
		ok = verifySaveData(saveFileName, data.getData(), data.size());

	if (ok)
		debug(1, "Wrote Zoombini2 savefile to %s", saveFileName.c_str());
	else
		warning("Zoombini2SavegameManager: Failed to write '%s'", saveFileName.c_str());
	return ok;
}

bool Zoombini2SavegameManager::loadSavefile(const Common::String &savefileName, GameState &state) const {
	if (!_saveFileManager || !isSafeStoredSavefileName(savefileName))
		return false;

	const Common::String saveFileName = makeSaveFileName(savefileName);
	Common::InSaveFile *stream = _saveFileManager->openForLoading(saveFileName);
	if (!stream) {
		warning("Zoombini2SavegameManager: Could not open '%s' for reading", saveFileName.c_str());
		return false;
	}

	const bool ok = state.load(stream);
	delete stream;
	if (ok)
		debug(1, "Loaded Zoombini2 savefile from %s", saveFileName.c_str());
	else
		warning("Zoombini2SavegameManager: Failed to load '%s'", saveFileName.c_str());
	return ok;
}

bool Zoombini2SavegameManager::importSavefile(const Common::String &savefileName, Common::SeekableReadStream *src, bool overwrite) const {
	if (!_saveFileManager || !src ||
		!isValidSavefileName(savefileName, _language) ||
		(!overwrite && savefileExists(savefileName)))
		return false;

	GameState importedState;
	if (!importedState.load(src)) {
		warning("Zoombini2SavegameManager: Failed to validate imported savefile '%s'", savefileName.c_str());
		return false;
	}

	// Z2 identifies an independent .mk file by its filename stem and embedded player name.
	importedState.setPlayerName(savefileName);
	return writeSavefile(savefileName, importedState);
}

bool Zoombini2SavegameManager::exportSavefile(const Common::String &savefileName, Common::WriteStream *dest) const {
	if (!_saveFileManager || !dest || !isSafeStoredSavefileName(savefileName))
		return false;

	GameState savedState;
	if (!loadSavefile(savefileName, savedState))
		return false;

	Common::InSaveFile *src = _saveFileManager->openForLoading(makeSaveFileName(savefileName));
	if (!src)
		return false;

	const int64 size = src->size();
	bool ok = 0 <= size;
	uint64 remaining = ok ? static_cast<uint64>(size) : 0;
	byte buffer[4096];
	while (ok && remaining != 0) {
		const uint32 amount = MIN<uint64>(sizeof(buffer), remaining);
		ok = src->read(buffer, amount) == amount && dest->write(buffer, amount) == amount;
		remaining -= amount;
	}
	ok = ok && !src->err() && !dest->err();
	delete src;
	return ok;
}

bool Zoombini2SavegameManager::deleteSavefile(const Common::String &savefileName) const {
	if (!_saveFileManager || !isSafeStoredSavefileName(savefileName))
		return false;
	return _saveFileManager->removeSavefile(makeSaveFileName(savefileName));
}

bool Zoombini2SavegameManager::renameSavefile(const Common::String &oldSavefileName, const Common::String &newSavefileName) const {
	if (!_saveFileManager ||
		!isSafeStoredSavefileName(oldSavefileName) ||
		!isValidSavefileName(newSavefileName, _language))
		return false;
	if (oldSavefileName == newSavefileName)
		return true;
	if (oldSavefileName.equalsIgnoreCase(newSavefileName) || savefileExists(newSavefileName))
		return false;

	GameState renamedState;
	if (!loadSavefile(oldSavefileName, renamedState))
		return false;
	renamedState.setPlayerName(newSavefileName);
	if (!writeSavefile(newSavefileName, renamedState))
		return false;
	if (deleteSavefile(oldSavefileName))
		return true;

	deleteSavefile(newSavefileName);
	return false;
}

bool Zoombini2SavegameManager::duplicateSavefile(const Common::String &srcSavefileName, const Common::String &newSavefileName) const {
	if (!_saveFileManager ||
		!isSafeStoredSavefileName(srcSavefileName) ||
		!isValidSavefileName(newSavefileName, _language) || srcSavefileName.equalsIgnoreCase(newSavefileName) || savefileExists(newSavefileName))
		return false;

	GameState duplicatedState;
	if (!loadSavefile(srcSavefileName, duplicatedState))
		return false;

	duplicatedState.setPlayerName(newSavefileName);
	return writeSavefile(newSavefileName, duplicatedState);
}

bool Zoombini2SavegameManager::savefileExists(const Common::String &savefileName) const {
	return _saveFileManager && isSafeStoredSavefileName(savefileName) && _saveFileManager->exists(makeSaveFileName(savefileName));
}

} // End of namespace Zoombini2
