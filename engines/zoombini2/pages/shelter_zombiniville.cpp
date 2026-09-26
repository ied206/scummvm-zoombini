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

#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/shelter_zombiniville.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr int ShelterZombiniville::kBoardingSlotCount;
constexpr const char *ShelterZombiniville::kBackgroundPath;
constexpr const char *ShelterZombiniville::kAreaMaskPath;
constexpr const char *ShelterZombiniville::kFeatureAnimationFormat;
constexpr const char *ShelterZombiniville::kOneRandomButtonPath;
constexpr const char *ShelterZombiniville::kAllRandomButtonPath;
constexpr const char *ShelterZombiniville::kCreateButtonPath;
constexpr const char *ShelterZombiniville::kBigZombAnimationPath;
constexpr const char *ShelterZombiniville::kLittleZombAnimationPath;
constexpr const char *ShelterZombiniville::kPickupZombAnimationPath;
constexpr const char *ShelterZombiniville::kIdleZombAnimationPath;
constexpr const char *ShelterZombiniville::kMusicPath;
constexpr const char *ShelterZombiniville::kFeatureSelectSoundPath;
constexpr const char *ShelterZombiniville::kOneRandomSoundPath;
constexpr const char *ShelterZombiniville::kAllRandomSoundPath;
constexpr const char *ShelterZombiniville::kValidZoombiniSoundPath;
constexpr const char *ShelterZombiniville::kWrongZoombiniSoundPath;

ShelterZombiniville::ShelterZombiniville(Zoombini2Engine *vm)
	: ShelterBase(vm) {
	_pageId = kPageZombiniville;
}

ShelterZombiniville::~ShelterZombiniville() {
	finishAllZoombiniReturns();
	if (_vm->isReturningToMap())
		_vm->_state->stashActiveZoombinis();
	delete _oneRandomButtonRunner;
	delete _allRandomButtonRunner;
	delete _createButtonRunner;
	delete _oneRandomButton;
	delete _allRandomButton;
	delete _createButton;
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++) {
			delete _featureButtonRunners[traitIdx][traitVal];
			delete _featureButtons[traitIdx][traitVal];
		}
	}

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		const int sounds[] = {
			_sndFeatureSelect,
			_sndOneRandom,
			_sndAllRandom,
			_sndValidZoombini,
			_sndWrongZoombini,
		};
		for (uint i = 0; i < ARRAYSIZE(sounds); i++) {
			if (0 <= sounds[i])
				sound->unload(sounds[i]);
		}
	}
}

const Common::Point32 &ShelterZombiniville::getFeatureDrawPosition(ZmbTrait::TraitKind traitKind, int traitVal) {
	static constexpr Common::Point32 kFeatureDrawPositions[ZmbTrait::kTraitKindCount][ZmbTrait::kTraitValueCount] = {
		{
			Common::Point32(237, 345),
			Common::Point32(292, 351),
			Common::Point32(398, 355),
			Common::Point32(346, 349),
			Common::Point32(438, 369),
		},
		{
			Common::Point32(534, 126),
			Common::Point32(529, 167),
			Common::Point32(526, 214),
			Common::Point32(522, 255),
			Common::Point32(518, 296),
		},
		{
			Common::Point32(229, 41),
			Common::Point32(286, 45),
			Common::Point32(404, 46),
			Common::Point32(345, 44),
			Common::Point32(453, 45),
		},
		{
			Common::Point32(162, 155),
			Common::Point32(165, 109),
			Common::Point32(163, 195),
			Common::Point32(157, 233),
			Common::Point32(157, 275),
		},
	};
	return kFeatureDrawPositions[static_cast<int>(traitKind)][traitVal];
}

const Common::Rect32 &ShelterZombiniville::getFeatureButtonRect(ZmbTrait::TraitKind traitKind, int traitVal) {
	static const Common::Rect32 kFeatureButtonRects[ZmbTrait::kTraitKindCount][ZmbTrait::kTraitValueCount] = {
		{
			Common::Rect32(238, 349, 286, 390),
			Common::Rect32(291, 352, 344, 396),
			Common::Rect32(398, 359, 433, 403),
			Common::Rect32(349, 353, 394, 398),
			Common::Rect32(438, 365, 478, 406),
		},
		{
			Common::Rect32(534, 118, 570, 151),
			Common::Rect32(529, 159, 564, 197),
			Common::Rect32(522, 207, 561, 247),
			Common::Rect32(518, 255, 557, 298),
			Common::Rect32(511, 303, 552, 341),
		},
		{
			Common::Rect32(226, 40, 276, 85),
			Common::Rect32(284, 42, 335, 87),
			Common::Rect32(403, 44, 448, 85),
			Common::Rect32(345, 43, 396, 87),
			Common::Rect32(454, 44, 501, 85),
		},
		{
			Common::Rect32(164, 153, 207, 186),
			Common::Rect32(162, 112, 203, 147),
			Common::Rect32(166, 190, 209, 223),
			Common::Rect32(166, 228, 212, 262),
			Common::Rect32(167, 271, 213, 304),
		},
	};
	return kFeatureButtonRects[static_cast<int>(traitKind)][traitVal];
}

void ShelterZombiniville::setupHoverRunners() {
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++) {
			AnimationRunner *runner = new AnimationRunner(_vm, getFeatureDrawPosition(traitKind, traitVal), AnimationRunnerMode::kPlayOnceAndHide02);
			runner->setAnimation(_featureButtons[traitIdx][traitVal]);
			runner->addTimedFrame(0, 60);
			runner->setHitRect(getFeatureButtonRect(traitKind, traitVal));
			_featureButtonRunners[traitIdx][traitVal] = runner;
		}
	}

	_oneRandomButtonRunner = new AnimationRunner(_vm, Common::Point32(131, 394), AnimationRunnerMode::kPlayOnceAndHide02);
	_oneRandomButtonRunner->setAnimation(_oneRandomButton);
	_oneRandomButtonRunner->addTimedFrame(1, 100);
	_oneRandomButtonRunner->addTimedFrame(2, 100);
	_oneRandomButtonRunner->addTimedFrame(1, 40);
	_oneRandomButtonRunner->setHitRect(_oneRandomRect);
	_oneRandomButtonRunner->setHitTestEnabled(true);

	_allRandomButtonRunner = new AnimationRunner(_vm, Common::Point32(230, 412), AnimationRunnerMode::kPlayOnceAndHide02);
	_allRandomButtonRunner->setAnimation(_allRandomButton);
	_allRandomButtonRunner->addTimedFrame(1, 100);
	_allRandomButtonRunner->addTimedFrame(2, 100);
	_allRandomButtonRunner->addTimedFrame(0, 100);
	_allRandomButtonRunner->setHitRect(_allRandomRect);
	_allRandomButtonRunner->setHitTestEnabled(true);

	_createButtonRunner = new AnimationRunner(_vm, Common::Point32(395, 429), AnimationRunnerMode::kPlayOnceAndHide02);
	_createButtonRunner->setAnimation(_createButton);
	_createButtonRunner->addTimedFrame(1, 100);
	_createButtonRunner->addTimedFrame(2, 100);
	_createButtonRunner->addTimedFrame(0, 100);
	_createButtonRunner->setHitRect(_createRect);
}

Common::Point32 ShelterZombiniville::getSlotPosition(uint index) {
	static constexpr Common::Point32 kSlotPos[kBoardingSlotCount] = {
		Common::Point32(490, 524),
		Common::Point32(500, 485),
		Common::Point32(452, 526),
		Common::Point32(455, 485),
		Common::Point32(410, 527),
		Common::Point32(408, 484),
		Common::Point32(370, 527),
		Common::Point32(366, 485),
		Common::Point32(327, 528),
		Common::Point32(327, 482),
		Common::Point32(286, 528),
		Common::Point32(285, 482),
		Common::Point32(246, 528),
		Common::Point32(248, 483),
		Common::Point32(209, 488),
		Common::Point32(177, 465),
	};
	return index < kBoardingSlotCount ? kSlotPos[index] : kSlotPos[kBoardingSlotCount - 1];
}

int ShelterZombiniville::findFirstFreeSlot() const {
	for (int slotIndex = 0; slotIndex < kBoardingSlotCount; slotIndex++) {
		if (!_boardingSlots[slotIndex].occupied)
			return slotIndex;
	}
	return -1;
}

void ShelterZombiniville::init() {
	debug(1, "ShelterZombiniville::init");
	_vm->_state->clearActiveZoombinis();
	_boardingZoombinis.clear();
	_departingZoombinis.clear();
	for (int slotIndex = 0; slotIndex < kBoardingSlotCount; slotIndex++)
		_boardingSlots[slotIndex].occupied = false;
	GameState *gameState = _vm->_state;
	if (_vm->_isSavedGame)
		gameState->restoreSavedZoombinis();
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		const int slotIndex = findFirstFreeSlot();
		if (slotIndex < 0) {
			warning("ShelterZombiniville: No free boarding slot for restored Zoombini %u", i);
			continue;
		}
		const Common::Point32 slotPos = getSlotPosition(slotIndex);
		zoombini->setPosition(slotPos);
		zoombini->_inputEnabled = true;
		zoombini->_placementIndex = slotIndex;
		_boardingSlots[slotIndex].occupied = true;
		_boardingZoombinis.push_back(zoombini);
	}
	refreshFeatureCounts();
	gameState->registerPageVisit(kPageZombiniville);

	if (!_vm->_gfx->loadBackground(kBackgroundPath))
		warning("ShelterZombiniville: Failed to load background");
	loadAreaMask(Common::Path(kAreaMaskPath));

	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++) {
			const Common::String path = Common::String::format(kFeatureAnimationFormat, traitIdx + 1, traitVal + 1);
			_featureButtons[traitIdx][traitVal] = new Animation(_vm);
			if (!_featureButtons[traitIdx][traitVal]->loadFromFile(Common::Path(path)))
				warning("ShelterZombiniville: Failed to load '%s'", path.c_str());
		}
	}

	_oneRandomButton = new Animation(_vm);
	_oneRandomButton->loadFromFile(Common::Path(kOneRandomButtonPath));
	_allRandomButton = new Animation(_vm);
	_allRandomButton->loadFromFile(Common::Path(kAllRandomButtonPath));
	_createButton = new Animation(_vm);
	_createButton->loadFromFile(Common::Path(kCreateButtonPath));

	_bigZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kBigZombAnimationPath), 0);
	if (!_bigZombAnimation)
		warning("ShelterZombiniville: Failed to load BigZomb.anm");
	_littleZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kLittleZombAnimationPath), 50);
	if (!_littleZombAnimation)
		warning("ShelterZombiniville: Failed to load littleZomb.anm");
	_pickupZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kPickupZombAnimationPath), 100);
	if (!_pickupZombAnimation)
		warning("ShelterZombiniville: Failed to load pris.anm");
	_idleZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kIdleZombAnimationPath), 50);
	if (!_idleZombAnimation)
		warning("ShelterZombiniville: Failed to load attenteZomb2.anm");
	for (uint i = 0; i < _boardingZoombinis.size(); i++)
		_boardingZoombinis[i]->setDefaultAnimation(_littleZombAnimation, 33);

	if (!_vm->_gfx->loadTextFont(Gfx::TextColor::kDark00))
		warning("ShelterZombiniville: Failed to load name font");

	setupHoverRunners();
	_vm->_gfx->getPageLayerStack()->setScrollLocked(true);
	resetSelectedFeatures();
	_currentName.clear();

	startPageMusic(Common::Path(kMusicPath));
	SoundManager *sound = _vm->getSoundManager();
	_sndFeatureSelect = sound->load(false, Common::Path(kFeatureSelectSoundPath), false);
	_sndOneRandom = sound->load(false, Common::Path(kOneRandomSoundPath), false);
	_sndAllRandom = sound->load(false, Common::Path(kAllRandomSoundPath), false);
	_sndValidZoombini = sound->load(false, Common::Path(kValidZoombiniSoundPath), false);
	_sndWrongZoombini = sound->load(false, Common::Path(kWrongZoombiniSoundPath), false);
}

void ShelterZombiniville::refreshFeatureCounts() {
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _boardingZoombinis[i];
		for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
			const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
			const byte traitVal = zoombini->_traits.getValue(traitKind);
			if (1 <= traitVal && traitVal <= ZmbTrait::kTraitValueCount)
				_featureCounts[traitIdx][traitVal] += 1;
		}
	}
}

void ShelterZombiniville::resetSelectedFeatures() {
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		_stations[traitIdx].selectedValue = 0;
		_vm->_selectedFeatures[traitIdx] = -1;
	}
}

void ShelterZombiniville::randomizeSelectedFeatures() {
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const byte traitVal = static_cast<byte>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		_stations[traitIdx].selectedValue = traitVal;
		_vm->_selectedFeatures[traitIdx] = traitVal;
	}
}

bool ShelterZombiniville::hasActiveEntrance() const {
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (!isZoombiniReturning(_boardingZoombinis[i]) && _boardingZoombinis[i]->_movementPath)
			return true;
	}
	return false;
}

bool ShelterZombiniville::hasActiveReturns() const {
	return !_departingZoombinis.empty();
}

bool ShelterZombiniville::isZoombiniReturning(const ZoombiniRunner *zoombini) const {
	for (uint i = 0; i < _departingZoombinis.size(); i++) {
		if (_departingZoombinis[i] == zoombini)
			return true;
	}
	return false;
}

bool ShelterZombiniville::canUseGoButton() const {
	return !hasActiveReturns() && _boardingZoombinis.size() == kBoardingSlotCount;
}

void ShelterZombiniville::onMapButtonPressed() {
	finishAllZoombiniReturns();
}

bool ShelterZombiniville::canCreateSelectedZoombini() const {
	if (hasActiveReturns() || findFirstFreeSlot() < 0 || kBoardingSlotCount <= _boardingZoombinis.size() || hasActiveEntrance())
		return false;
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		if (_stations[traitIdx].selectedValue == 0)
			return false;
	}
	return true;
}

bool ShelterZombiniville::passesPackTraitLimits(const ZmbTrait &traits) const {
	int counts[ZmbTrait::kTraitKindCount][ZmbTrait::kTraitValueCount + 1] = {};
	if (!traits.hasValidValues())
		return false;
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
		counts[traitIdx][traits.getValue(traitKind)] += 1;
	}

	const uint16 candidateHash = traits.calculateHash();
	bool hasCandidateMatch = false;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _boardingZoombinis[i];
		for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
			const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
			const byte traitVal = zoombini->_traits.getValue(traitKind);
			if (traitVal < 1 || ZmbTrait::kTraitValueCount < traitVal)
				return false;
			counts[traitIdx][traitVal] += 1;
		}
		if (zoombini->_traitHash == candidateHash)
			hasCandidateMatch = true;
	}

	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 1; traitVal <= ZmbTrait::kTraitValueCount; traitVal++) {
			if (5 < counts[traitIdx][traitVal])
				return false;
		}
	}
	if (!hasCandidateMatch)
		return true;

	// Count existing members against themselves and later members.
	int equalRosterPairs = 0;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		for (uint j = i; j < _boardingZoombinis.size(); j++) {
			if (_boardingZoombinis[i]->_traitHash == _boardingZoombinis[j]->_traitHash) {
				equalRosterPairs += 1;
				if (2 < equalRosterPairs)
					return false;
			}
		}
	}
	return true;
}

Common::String ShelterZombiniville::generateName() {
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
	const int targetLength = _vm->_rnd->getRandomNumber(1) + 4;
	bool useVowelPair = _vm->_rnd->getRandomNumber(98) + 1 < 40;
	int length = 0;
	while (length < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[_vm->_rnd->getRandomNumber(29)];
			if (pair[1] != ' ') {
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
			} else {
				name[length] = pair[0];
				length += 1;
			}
		} else {
			useVowelPair = true;
			if (1 < length || _vm->_rnd->getRandomNumber(98) + 1 <= 33) {
				const char *pair = kConsonantPairs[_vm->_rnd->getRandomNumber(38)];
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
				usedConsonantPair = true;
			} else {
				name[length] = kSingleConsonants[_vm->_rnd->getRandomNumber(30)];
				length += 1;
			}
		}
		if (usedConsonantPair && targetLength <= length)
			name[length - 1] = kEndings[_vm->_rnd->getRandomNumber(4)];
		if (length == 2 && name[0] == name[1])
			length = 1;
	}
	return Common::String(name);
}

PathObject *ShelterZombiniville::createEntrancePath(const Common::Point32 &dest) const {
	PathObject *path = new PathObject(_vm);
	path->appendSegment(Common::Point32(-70, 400), Common::Point32(dest.x / 3, 420), Common::Point32(2 * (dest.x / 3), 450), dest, 2, 0);
	path->endPos = dest;
	return path;
}

PathObject *ShelterZombiniville::createReturnPath(const Common::Point32 &start) const {
	static constexpr int kReturnStepValue = 4;
	const Common::Point32 dest(-70, 400);
	PathObject *path = new PathObject(_vm);
	path->appendSegment(start, Common::Point32(2 * (start.x / 3), 450), Common::Point32(start.x / 3, 420), dest, kReturnStepValue, 0);
	path->endPos = dest;
	return path;
}

void ShelterZombiniville::restoreSelectedFeatures(const ZoombiniRunner &zoombini) {
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
		const byte traitVal = zoombini._traits.getValue(traitKind);
		_stations[traitIdx].selectedValue = traitVal;
		_vm->_selectedFeatures[traitIdx] = traitVal;
	}
}

void ShelterZombiniville::sendZoombiniOff(ZoombiniRunner *zoombini, bool restoreFeatures) {
	if (!zoombini || isZoombiniReturning(zoombini))
		return;

	const int slotIndex = zoombini->_placementIndex;
	if (slotIndex < 0 || kBoardingSlotCount <= slotIndex || !_boardingSlots[slotIndex].occupied) {
		warning("ShelterZombiniville: Cannot return Zoombini from invalid slot %d", slotIndex);
		return;
	}

	if (restoreFeatures)
		restoreSelectedFeatures(*zoombini);
	zoombini->endDrag(true);
	zoombini->_inputEnabled = false;
	_boardingSlots[slotIndex].occupied = false;
	_departingZoombinis.push_back(zoombini);
	const uint32 tick = _vm->getGameTickCount();
	zoombini->startDirectionTrackedAnimation(tick);
	zoombini->startMovement(createReturnPath(zoombini->_screenPos), tick);
	debug(2, "ShelterZombiniville: Returning '%s' from slot %d", zoombini->_name, slotIndex);
}

void ShelterZombiniville::sendAllZoombinisOff() {
	const Common::Array<ZoombiniRunner *> zoombinis = _boardingZoombinis;
	for (uint i = 0; i < zoombinis.size(); i++)
		sendZoombiniOff(zoombinis[i], false);
}

void ShelterZombiniville::finishSendingZoombiniOff(ZoombiniRunner *zoombini) {
	int departingIndex = -1;
	for (uint i = 0; i < _departingZoombinis.size(); i++) {
		if (_departingZoombinis[i] == zoombini) {
			departingIndex = static_cast<int>(i);
			break;
		}
	}
	if (departingIndex < 0)
		return;

	_departingZoombinis.remove_at(departingIndex);
	zoombini->clearMovement();
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i] == zoombini) {
			_boardingZoombinis.remove_at(i);
			break;
		}
	}
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		if (_vm->_state->_activeZoombinis[i] == zoombini) {
			_vm->_state->_activeZoombinis.remove_at(i);
			break;
		}
	}
	if (!_vm->_state->_traitComboTable.unregisterCombo(zoombini->_traits))
		warning("ShelterZombiniville: Failed to unregister returned Zoombini traits %s", zoombini->_traits.toStr().c_str());
	debug(2, "ShelterZombiniville: Returned '%s'", zoombini->_name);
	delete zoombini;
	refreshFeatureCounts();
}

void ShelterZombiniville::finishAllZoombiniReturns() {
	while (!_departingZoombinis.empty())
		finishSendingZoombiniOff(_departingZoombinis.back());
}

bool ShelterZombiniville::createZoombini(bool allowConcurrentEntrances) {
	if (hasActiveReturns() || (!allowConcurrentEntrances && hasActiveEntrance()) || kBoardingSlotCount <= _boardingZoombinis.size())
		return false;
	const int slotIndex = findFirstFreeSlot();
	if (slotIndex < 0)
		return false;

	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		if (_stations[traitIdx].selectedValue == 0)
			return false;
	}
	const ZmbTrait traits(_stations[0].selectedValue, _stations[1].selectedValue, _stations[2].selectedValue, _stations[3].selectedValue);

	GameState *gameState = _vm->_state;
	if (gameState->hasReachedZoombiniRegistrationLimit() || !gameState->_traitComboTable.canRegisterCombo(traits))
		return false;
	if (!gameState->hasRelaxedPackTraitLimits() && !passesPackTraitLimits(traits))
		return false;
	if (!gameState->_traitComboTable.registerCombo(traits))
		return false;

	const Common::Point32 dest = getSlotPosition(slotIndex);
	ZoombiniRunner *zoombini = new ZoombiniRunner();
	zoombini->setTraits(traits);
	_currentName = generateName();
	Common::strlcpy(zoombini->_name, _currentName.c_str(), sizeof(zoombini->_name));
	zoombini->setPosition(dest);
	zoombini->setDefaultAnimation(_littleZombAnimation, 33);
	zoombini->_inputEnabled = false;
	zoombini->_placementIndex = slotIndex;
	zoombini->_tracksMovementDirection = true;
	const uint32 tick = _vm->getGameTickCount();
	zoombini->startAnimation(nullptr, 66, tick);
	zoombini->startMovement(createEntrancePath(dest), tick);
	_vm->_state->_activeZoombinis.push_back(zoombini);
	_boardingZoombinis.push_back(zoombini);
	_boardingSlots[slotIndex].occupied = true;
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
		_featureCounts[traitIdx][traits.getValue(traitKind)] += 1;
	}

	debug(2, "ShelterZombiniville: Created '%s' with traits %s in slot %d", _currentName.c_str(), traits.toStr().c_str(), slotIndex);
	return true;
}

void ShelterZombiniville::onUpdate() {
	const uint32 tick = _vm->getGameTickCount();
	for (int i = static_cast<int>(_departingZoombinis.size()) - 1; 0 <= i; i--) {
		ZoombiniRunner *zoombini = _departingZoombinis[i];
		zoombini->updateAnimation(tick);
		if (!zoombini->advanceMovement(tick))
			finishSendingZoombiniOff(zoombini);
	}

	bool completedEntrance = false;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _boardingZoombinis[i];
		if (isZoombiniReturning(zoombini))
			continue;
		if (zoombini->_movementPath && !zoombini->advanceMovement(tick)) {
			zoombini->clearMovement();
			zoombini->_inputEnabled = true;
			zoombini->resetAnimation();
			completedEntrance = true;
		}
		zoombini->updateAnimation(tick);
	}
	if (completedEntrance)
		resetSelectedFeatures();
}

void ShelterZombiniville::buildBoardingZoombiniDrawOrder(Common::Array<uint> &order) const {
	order.clear();
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i]->_dragging)
			continue;
		order.push_back(i);
	}
	ZoombiniRunner::sortDrawOrderByY(_boardingZoombinis, order);
}

void ShelterZombiniville::drawBoardingZoombinis(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	buildBoardingZoombiniDrawOrder(order);
	for (uint i = 0; i < order.size(); i++)
		_vm->_gfx->drawZoombiniRunner(screen, _boardingZoombinis[order[i]]);
}

ZoombiniRunner *ShelterZombiniville::getDraggedZoombini() const {
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i]->_dragging)
			return _boardingZoombinis[i];
	}
	return nullptr;
}

void ShelterZombiniville::onRenderContent(ManagedSurface32 *screen) {
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));
	drawHoverRunners(screen);

	if (hasActiveEntrance() && _vm->_gfx->hasTextFont(Gfx::TextColor::kDark00)) {
		const int width = _vm->_gfx->getTextWidth(_currentName, Gfx::TextColor::kDark00);
		_vm->_gfx->drawText(screen, Gfx::TextColor::kDark00, Common::Point32(367 - width / 2, 280), _currentName);
	}
}

void ShelterZombiniville::onRenderActors(ManagedSurface32 *screen) {
	drawBoardingZoombinis(screen);
}

void ShelterZombiniville::onActorsRendered() {
	const uint32 tick = _vm->getGameTickCount();
	Common::Array<uint> order;
	buildBoardingZoombiniDrawOrder(order);
	for (uint i = 0; i < order.size(); i++) {
		ZoombiniRunner *zoombini = _boardingZoombinis[order[i]];
		if (zoombini->_hidden)
			continue;
		zoombini->tryStartIdleAnimation(_idleZombAnimation, *_vm->_rnd, tick, _vm->getFrameDeltaMs(), _vm->getLogicPacingHz());
		zoombini->advanceAnimationAfterDraw();
	}
}

void ShelterZombiniville::onRenderForeground(ManagedSurface32 *screen) {
	byte selectedValues[ZmbTrait::kTraitKindCount];
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++)
		selectedValues[traitIdx] = _stations[traitIdx].selectedValue;
	_vm->_gfx->drawZoombiniPreview(screen, _bigZombAnimation, selectedValues, Common::Point32(300, 110));
}

void ShelterZombiniville::renderDragOverlay(ManagedSurface32 *screen, bool advanceState) {
	ZoombiniRunner *draggedZoombini = getDraggedZoombini();
	if (!draggedZoombini || draggedZoombini->_hidden)
		return;

	_vm->_gfx->drawZoombiniRunner(screen, draggedZoombini);
	if (advanceState)
		draggedZoombini->advanceAnimationAfterDraw();
}

void ShelterZombiniville::drawHoverRunners(ManagedSurface32 *screen) {
	const uint32 tick = _vm->getGameTickCount();
	for (byte traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
		for (byte traitVal = 1; traitVal <= ZmbTrait::kTraitValueCount; traitVal++) {
			_vm->_gfx->drawAndUpdateAnimationRunner(screen, _featureButtonRunners[traitIdx][traitVal - 1], tick, 0, ManagedSurface32::kScreenSize.width);
			if (traitKind == ZmbTrait::TraitKind::kNose01) {
				recolorPickerNose(screen, traitVal + 1);
			}
		}
	}
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _oneRandomButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _allRandomButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _createButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
}

void ShelterZombiniville::recolorPickerNose(ManagedSurface32 *screen, byte noseVal) const {
	if (!screen || noseVal < 3)
		return;
	RGBColor targetColor;
	if (!Gfx::noseColorRGB(_vm->getColorAssistMode(), noseVal, targetColor))
		return;

	const Common::Point32 &origin = getFeatureDrawPosition(ZmbTrait::TraitKind::kNose01, noseVal - 1);
	const int centerX = origin.x + 16;
	const int centerY = origin.y + 17;
	for (int dy = -12; dy <= 12; dy++) {
		const int y = centerY + dy;
		if (y < 0 || screen->h <= y)
			continue;
		for (int dx = -12; dx <= 12; dx++) {
			const int x = centerX + dx;
			if (x < 0 || screen->w <= x || 144 < dx * dx + dy * dy)
				continue;
			byte *pixel = static_cast<byte *>(screen->getBasePtr(x, y));
			const RGBColor sourceColor = RGBColor::fromBGR(pixel);
			bool matchingColor = false;
			if (noseVal == 3)
				matchingColor = sourceColor.r + 12 < sourceColor.g && sourceColor.b + 12 < sourceColor.g;
			else if (noseVal == 4)
				matchingColor = sourceColor.r + 20 < sourceColor.b && sourceColor.g + 20 < sourceColor.b;
			else if (noseVal == 5)
				matchingColor = sourceColor.g + 15 < sourceColor.r && sourceColor.g + 15 < sourceColor.b;
			if (!matchingColor)
				continue;
			const RGBColor adjustedColor = Gfx::recolorNoseGradientRGB(sourceColor, targetColor);
			adjustedColor.writeBGR(pixel);
		}
	}
	screen->addDirtyRect(Common::Rect(centerX - 12, centerY - 12, centerX + 13, centerY + 13));
}

void ShelterZombiniville::updateHoverRunners() {
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++)
			_featureButtonRunners[traitIdx][traitVal]->setHitTestEnabled(_featureCounts[traitIdx][traitVal + 1] < 5);
	}
	// Quick Fill and Batch Fill keep the enabled flag from setup, mirroring
	// the original, which never refreshes them after SetRect.
	_createButtonRunner->setHitTestEnabled(canCreateSelectedZoombini());

	if (!_vm->_gfx->getPageLayerStack()->isScrollLocked() || getDraggedZoombini())
		return;

	const Common::Point32 mousePos = _vm->getMousePos();
	const uint32 tick = _vm->getGameTickCount();
	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++) {
			AnimationRunner *runner = _featureButtonRunners[traitIdx][traitVal];
			if (runner->isHitTestEnabled() && runner->containsHitPoint(mousePos))
				runner->reset(tick);
		}
	}
	// The quick/batch/create overlays are deliberately excluded here. In the
	// original (ShelterZombiniville__Update_43C880) the hover loop covers only
	// the 20 trait runners; the bumper overlays start exclusively on press
	// through PageLayer__ActivateRunnersAt_459E70.
}

void ShelterZombiniville::onPostRender() {
	updateHoverRunners();
}

EventHandleResult ShelterZombiniville::onLButtonDown(const Common::Point &pos) {
	if (getDraggedZoombini())
		return EventHandleResult::kConsumed;

	for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
		for (int traitVal = 0; traitVal < ZmbTrait::kTraitValueCount; traitVal++) {
			if (_featureButtonRunners[traitIdx][traitVal]->containsHitPoint(Common::Point32(pos))) {
				if (hasActiveEntrance() || hasActiveReturns() || 5 <= _featureCounts[traitIdx][traitVal + 1])
					return EventHandleResult::kConsumed;
				const byte selectedValue = static_cast<byte>(traitVal + 1);
				_stations[traitIdx].selectedValue = selectedValue;
				_vm->_selectedFeatures[traitIdx] = selectedValue;
				if (0 <= _sndFeatureSelect)
					_vm->getSoundManager()->playWithVolume(_sndFeatureSelect, _vm->getSoundManager()->_volumeSFX);
				return EventHandleResult::kConsumed;
			}
		}
	}

	if (_createButtonRunner->containsHitPoint(Common::Point32(pos))) {
		// The bumper overlays are not layer-registered, so start them here: the
		// original starts them on every press through ActivateRunnersAt.
		_createButtonRunner->start(_vm->getGameTickCount());
		if (!canCreateSelectedZoombini() || !createZoombini(false)) {
			if (0 <= _sndWrongZoombini)
				_vm->getSoundManager()->playWithVolume(_sndWrongZoombini, _vm->getSoundManager()->_volumeSFX);
			return EventHandleResult::kConsumed;
		}
		if (0 <= _sndValidZoombini)
			_vm->getSoundManager()->playWithVolume(_sndValidZoombini, _vm->getSoundManager()->_volumeSFX);
		if (_boardingZoombinis.size() == kBoardingSlotCount)
			_vm->restartGoBlink();
		if (_vm->_state->hasReachedZoombiniRegistrationLimit())
			_vm->restartGoBlink();
		return EventHandleResult::kConsumed;
	}

	if (_oneRandomButtonRunner->containsHitPoint(Common::Point32(pos))) {
		_oneRandomButtonRunner->start(_vm->getGameTickCount());
		if (_boardingZoombinis.size() < kBoardingSlotCount && !hasActiveEntrance() && !hasActiveReturns()) {
			if (0 <= _sndOneRandom)
				_vm->getSoundManager()->playWithVolume(_sndOneRandom, _vm->getSoundManager()->_volumeSFX);
			if (_vm->_state->hasReachedZoombiniRegistrationLimit()) {
				_vm->restartGoBlink();
				return EventHandleResult::kConsumed;
			}
			do {
				randomizeSelectedFeatures();
			} while (!createZoombini(false));
			if (_boardingZoombinis.size() == kBoardingSlotCount)
				_vm->restartGoBlink();
		}
		return EventHandleResult::kConsumed;
	}

	if (_allRandomButtonRunner->containsHitPoint(Common::Point32(pos))) {
		_allRandomButtonRunner->start(_vm->getGameTickCount());
		if (_boardingZoombinis.size() < kBoardingSlotCount && !hasActiveEntrance() && !hasActiveReturns()) {
			if (0 <= _sndAllRandom)
				_vm->getSoundManager()->playWithVolume(_sndAllRandom, _vm->getSoundManager()->_volumeSFX);
			randomizeSelectedFeatures();
			while (_boardingZoombinis.size() < kBoardingSlotCount) {
				if (_vm->_state->hasReachedZoombiniRegistrationLimit())
					break;
				do {
					randomizeSelectedFeatures();
				} while (!createZoombini(true));
			}
			if (!_boardingZoombinis.empty()) {
				const ZoombiniRunner *last = _boardingZoombinis.back();
				for (int traitIdx = 0; traitIdx < ZmbTrait::kTraitKindCount; traitIdx++) {
					const ZmbTrait::TraitKind traitKind = static_cast<ZmbTrait::TraitKind>(traitIdx);
					const byte traitVal = last->_traits.getValue(traitKind);
					_stations[traitIdx].selectedValue = traitVal;
					_vm->_selectedFeatures[traitIdx] = traitVal;
				}
			}
			_vm->restartGoBlink();
		}
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterZombiniville::onLButtonUp(const Common::Point &pos) {
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_boardingZoombinis, Common::Point32(pos.x, pos.y), true,
																	_pickupZombAnimation, _vm->getGameTickCount(), nullptr, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult ShelterZombiniville::onMouseMove(const Common::Point &pos) {
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_boardingZoombinis, Common::Point32(pos.x, pos.y), false,
																	_pickupZombAnimation, _vm->getGameTickCount(), nullptr, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult ShelterZombiniville::onKeyDown(const Common::KeyState &key, bool repeat) {
	if (!_vm->useEnhancedKbdShortcuts() || key.keycode != Common::KEYCODE_DELETE)
		return EventHandleResult::kPassthrough;

	if (!repeat) {
		if (key.hasFlags(Common::KBD_SHIFT))
			sendAllZoombinisOff();
		else
			sendZoombiniOff(getDraggedZoombini(), true);
	}
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
