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

constexpr const char *ShelterZombiniville::kBackgroundPath;
constexpr const char *ShelterZombiniville::kAreaMaskPath;
constexpr const char *ShelterZombiniville::kFeatureAnimationFormat;
constexpr const char *ShelterZombiniville::kQuickFillButtonPath;
constexpr const char *ShelterZombiniville::kBatchFillButtonPath;
constexpr const char *ShelterZombiniville::kCreateButtonPath;
constexpr const char *ShelterZombiniville::kBigZombAnimationPath;
constexpr const char *ShelterZombiniville::kLittleZombAnimationPath;
constexpr const char *ShelterZombiniville::kPickupZombAnimationPath;
constexpr const char *ShelterZombiniville::kIdleZombAnimationPath;
constexpr const char *ShelterZombiniville::kMusicPath;
constexpr const char *ShelterZombiniville::kFeatureSelectSoundPath;
constexpr const char *ShelterZombiniville::kQuickFillSoundPath;
constexpr const char *ShelterZombiniville::kBatchFillSoundPath;
constexpr const char *ShelterZombiniville::kValidZoombiniSoundPath;
constexpr const char *ShelterZombiniville::kWrongZoombiniSoundPath;

ShelterZombiniville::ShelterZombiniville(Zoombini2Engine *vm)
	: ShelterBase(vm) {
	_pageId = kPageZombiniville;
}

ShelterZombiniville::~ShelterZombiniville() {
	if (_vm->isReturningToMap())
		GameState::transferSavedRoster(_vm->_globalZoombinis, _vm->getGameState()->_savedRoster);
	delete _quickFillButtonRunner;
	delete _batchFillButtonRunner;
	delete _createButtonRunner;
	delete _quickFillButton;
	delete _batchFillButton;
	delete _createButton;
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			delete _featureButtonRunners[feature][value];
			delete _featureButtons[feature][value];
		}
	}

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		const int sounds[] = {_musicId, _sndFeatureSelect, _sndQuickFill, _sndBatchFill, _sndValidZoombini, _sndWrongZoombini};
		for (uint i = 0; i < ARRAYSIZE(sounds); i++) {
			if (0 <= sounds[i])
				sound->unload(sounds[i]);
		}
	}
}

void ShelterZombiniville::setupFeatureRects() {
	// Bottom station: feet.
	_stations[0].drawPos[0] = Common::Point32(237, 345);
	_stations[0].drawPos[1] = Common::Point32(292, 351);
	_stations[0].drawPos[2] = Common::Point32(398, 355);
	_stations[0].drawPos[3] = Common::Point32(346, 349);
	_stations[0].drawPos[4] = Common::Point32(438, 369);
	_stations[0].buttonRects[0] = Common::Rect32(238, 349, 286, 390);
	_stations[0].buttonRects[1] = Common::Rect32(291, 352, 344, 396);
	_stations[0].buttonRects[2] = Common::Rect32(398, 359, 433, 403);
	_stations[0].buttonRects[3] = Common::Rect32(349, 353, 394, 398);
	_stations[0].buttonRects[4] = Common::Rect32(438, 365, 478, 406);

	// Right station: noses.
	_stations[1].drawPos[0] = Common::Point32(534, 126);
	_stations[1].drawPos[1] = Common::Point32(529, 167);
	_stations[1].drawPos[2] = Common::Point32(526, 214);
	_stations[1].drawPos[3] = Common::Point32(522, 255);
	_stations[1].drawPos[4] = Common::Point32(518, 296);
	_stations[1].buttonRects[0] = Common::Rect32(534, 118, 570, 151);
	_stations[1].buttonRects[1] = Common::Rect32(529, 159, 564, 197);
	_stations[1].buttonRects[2] = Common::Rect32(522, 207, 561, 247);
	_stations[1].buttonRects[3] = Common::Rect32(518, 255, 557, 298);
	_stations[1].buttonRects[4] = Common::Rect32(511, 303, 552, 341);

	// Top station: hair.
	_stations[2].drawPos[0] = Common::Point32(229, 41);
	_stations[2].drawPos[1] = Common::Point32(286, 45);
	_stations[2].drawPos[2] = Common::Point32(404, 46);
	_stations[2].drawPos[3] = Common::Point32(345, 44);
	_stations[2].drawPos[4] = Common::Point32(453, 45);
	_stations[2].buttonRects[0] = Common::Rect32(226, 40, 276, 85);
	_stations[2].buttonRects[1] = Common::Rect32(284, 42, 335, 87);
	_stations[2].buttonRects[2] = Common::Rect32(403, 44, 448, 85);
	_stations[2].buttonRects[3] = Common::Rect32(345, 43, 396, 87);
	_stations[2].buttonRects[4] = Common::Rect32(454, 44, 501, 85);

	// Left station: eyes.
	_stations[3].drawPos[0] = Common::Point32(162, 155);
	_stations[3].drawPos[1] = Common::Point32(165, 109);
	_stations[3].drawPos[2] = Common::Point32(163, 195);
	_stations[3].drawPos[3] = Common::Point32(157, 233);
	_stations[3].drawPos[4] = Common::Point32(157, 275);
	_stations[3].buttonRects[0] = Common::Rect32(164, 153, 207, 186);
	_stations[3].buttonRects[1] = Common::Rect32(162, 112, 203, 147);
	_stations[3].buttonRects[2] = Common::Rect32(166, 190, 209, 223);
	_stations[3].buttonRects[3] = Common::Rect32(166, 228, 212, 262);
	_stations[3].buttonRects[4] = Common::Rect32(167, 271, 213, 304);

	_quickFillRect = Common::Rect32(135, 399, 211, 448);
	_batchFillRect = Common::Rect32(239, 416, 318, 477);
	_createRect = Common::Rect32(404, 438, 491, 500);
}

void ShelterZombiniville::setupHoverRunners() {
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			AnimationRunner *runner = new AnimationRunner(_vm, _stations[feature].drawPos[value], AnimationRunnerMode::kPlayOnceAndHide02);
			runner->setAnimation(_featureButtons[feature][value]);
			runner->addTimedFrame(0, 60);
			runner->setHitRect(_stations[feature].buttonRects[value]);
			_featureButtonRunners[feature][value] = runner;
		}
	}

	_quickFillButtonRunner = new AnimationRunner(_vm, Common::Point32(131, 394), AnimationRunnerMode::kPlayOnceAndHide02);
	_quickFillButtonRunner->setAnimation(_quickFillButton);
	_quickFillButtonRunner->addTimedFrame(1, 100);
	_quickFillButtonRunner->addTimedFrame(2, 100);
	_quickFillButtonRunner->addTimedFrame(1, 40);
	_quickFillButtonRunner->setHitRect(_quickFillRect);
	// The original enables hit testing in SetRect and never refreshes the
	// bumper flags, so the hover cursor stays available over these buttons.
	_quickFillButtonRunner->setHitTestEnabled(true);

	_batchFillButtonRunner = new AnimationRunner(_vm, Common::Point32(230, 412), AnimationRunnerMode::kPlayOnceAndHide02);
	_batchFillButtonRunner->setAnimation(_batchFillButton);
	_batchFillButtonRunner->addTimedFrame(1, 100);
	_batchFillButtonRunner->addTimedFrame(2, 100);
	_batchFillButtonRunner->addTimedFrame(0, 100);
	_batchFillButtonRunner->setHitRect(_batchFillRect);
	_batchFillButtonRunner->setHitTestEnabled(true);

	_createButtonRunner = new AnimationRunner(_vm, Common::Point32(395, 429), AnimationRunnerMode::kPlayOnceAndHide02);
	_createButtonRunner->setAnimation(_createButton);
	_createButtonRunner->addTimedFrame(1, 100);
	_createButtonRunner->addTimedFrame(2, 100);
	_createButtonRunner->addTimedFrame(0, 100);
	_createButtonRunner->setHitRect(_createRect);
}

Common::Point32 ShelterZombiniville::getSlotPosition(uint index) {
	static constexpr Common::Point32 kSlotPos[kMaxPackSize] = {
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
	return index < static_cast<uint>(kMaxPackSize) ? kSlotPos[index] : kSlotPos[kMaxPackSize - 1];
}

void ShelterZombiniville::init() {
	debug(1, "ShelterZombiniville::init");
	_vm->clearGlobalZoombinis();
	_boardingZoombinis.clear();
	GameState *gameState = _vm->getGameState();
	if (_vm->_isSavedGame)
		GameState::transferSavedRoster(gameState->_savedRoster, _vm->_globalZoombinis);
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		const Common::Point32 slotPos = getSlotPosition(i);
		zoombini->setPosition(slotPos);
		zoombini->_inputEnabled = true;
		zoombini->_placementIndex = i;
		_boardingZoombinis.push_back(zoombini);
	}
	refreshFeatureCounts();
	gameState->registerPageVisit(kPageZombiniville);

	if (!_vm->_gfx->loadBackground(Common::Path(kBackgroundPath)))
		warning("ShelterZombiniville: Failed to load background");
	loadAreaMask(Common::Path(kAreaMaskPath));

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			const Common::String path = Common::String::format(kFeatureAnimationFormat, feature + 1, value + 1);
			_featureButtons[feature][value] = new Animation(_vm);
			if (!_featureButtons[feature][value]->loadFromFile(Common::Path(path)))
				warning("ShelterZombiniville: Failed to load '%s'", path.c_str());
		}
	}

	_quickFillButton = new Animation(_vm);
	_quickFillButton->loadFromFile(Common::Path(kQuickFillButtonPath));
	_batchFillButton = new Animation(_vm);
	_batchFillButton->loadFromFile(Common::Path(kBatchFillButtonPath));
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

	setupFeatureRects();
	setupHoverRunners();
	_vm->_gfx->getPageLayerStack()->setScrollLocked(true);
	resetSelectedFeatures();
	_currentName.clear();

	SoundManager *sound = _vm->getSoundManager();
	_musicId = sound->load(true, Common::Path(kMusicPath), true);
	if (0 <= _musicId)
		sound->playLoop(_musicId);
	_sndFeatureSelect = sound->load(false, Common::Path(kFeatureSelectSoundPath), false);
	_sndQuickFill = sound->load(false, Common::Path(kQuickFillSoundPath), false);
	_sndBatchFill = sound->load(false, Common::Path(kBatchFillSoundPath), false);
	_sndValidZoombini = sound->load(false, Common::Path(kValidZoombiniSoundPath), false);
	_sndWrongZoombini = sound->load(false, Common::Path(kWrongZoombiniSoundPath), false);
}

void ShelterZombiniville::refreshFeatureCounts() {
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _boardingZoombinis[i];
		for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
			const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(feature);
			const byte value = zoombini->_traits.getValue(traitIndex);
			if (1 <= value && value <= ZmbTrait::kTraitValueCount)
				_featureCounts[feature][value] += 1;
		}
	}
}

void ShelterZombiniville::resetSelectedFeatures() {
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		_stations[feature].selectedValue = 0;
		_vm->_selectedFeatures[feature] = -1;
	}
}

void ShelterZombiniville::randomizeSelectedFeatures() {
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		const int value = _vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1;
		_stations[feature].selectedValue = value;
		_vm->_selectedFeatures[feature] = value;
	}
}

bool ShelterZombiniville::hasActiveEntrance() const {
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i]->_movementPath)
			return true;
	}
	return false;
}

bool ShelterZombiniville::canUseGoButton() const {
	return static_cast<int>(_boardingZoombinis.size()) == kMaxPackSize;
}

bool ShelterZombiniville::canCreateSelectedZoombini() const {
	if (kMaxPackSize <= static_cast<int>(_boardingZoombinis.size()) || hasActiveEntrance())
		return false;
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		if (_stations[feature].selectedValue == 0)
			return false;
	}
	return true;
}

bool ShelterZombiniville::passesPackTraitLimits(const ZmbTrait &traits) const {
	int counts[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount + 1] = {};
	if (!traits.hasValidValues())
		return false;
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(feature);
		counts[feature][traits.getValue(traitIndex)] += 1;
	}

	const uint16 candidateHash = traits.calculateHash();
	int matchingCombinations = 0;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _boardingZoombinis[i];
		for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
			const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(feature);
			const byte value = zoombini->_traits.getValue(traitIndex);
			if (value < 1 || ZmbTrait::kTraitValueCount < value)
				return false;
			counts[feature][value] += 1;
		}
		if (zoombini->_traitHash == candidateHash)
			matchingCombinations += 1;
	}

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 1; value <= ZmbTrait::kTraitValueCount; value++) {
			if (5 < counts[feature][value])
				return false;
		}
	}
	return matchingCombinations < 2;
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

bool ShelterZombiniville::createZoombini(bool allowConcurrentEntrances) {
	if ((!allowConcurrentEntrances && hasActiveEntrance()) || kMaxPackSize <= static_cast<int>(_boardingZoombinis.size()))
		return false;

	for (int traitIndex = 0; traitIndex < ZmbTrait::kTraitCount; traitIndex++) {
		if (_stations[traitIndex].selectedValue == 0)
			return false;
	}
	const ZmbTrait traits(static_cast<byte>(_stations[0].selectedValue), static_cast<byte>(_stations[1].selectedValue),
						  static_cast<byte>(_stations[2].selectedValue), static_cast<byte>(_stations[3].selectedValue));

	GameState *gameState = _vm->getGameState();
	if (gameState->hasReachedZoombiniRegistrationLimit() || !gameState->canRegisterTraits(traits))
		return false;
	if (!gameState->hasRelaxedPackTraitLimits() && !passesPackTraitLimits(traits))
		return false;
	if (!gameState->registerTraits(traits))
		return false;

	const Common::Point32 dest = getSlotPosition(_boardingZoombinis.size());
	ZoombiniRunner *zoombini = new ZoombiniRunner();
	zoombini->setTraits(traits);
	_currentName = generateName();
	Common::strlcpy(zoombini->_name, _currentName.c_str(), sizeof(zoombini->_name));
	zoombini->setPosition(dest);
	zoombini->setDefaultAnimation(_littleZombAnimation, 33);
	zoombini->_inputEnabled = false;
	zoombini->_placementIndex = _boardingZoombinis.size();
	zoombini->_tracksMovementDirection = true;
	const uint32 tick = _vm->getGameTickCount();
	zoombini->startAnimation(nullptr, 66, tick);
	zoombini->startMovement(createEntrancePath(dest), tick);
	_vm->_globalZoombinis.push_back(zoombini);
	_boardingZoombinis.push_back(zoombini);
	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitCount; traitOrdinal++) {
		const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(traitOrdinal);
		_featureCounts[traitOrdinal][traits.getValue(traitIndex)] += 1;
	}

	debug(2, "ShelterZombiniville: Created '%s' with traits %s in slot %u", _currentName.c_str(), traits.toStr().c_str(), _boardingZoombinis.size() - 1);
	return true;
}

void ShelterZombiniville::onUpdate() {
	const uint32 tick = _vm->getGameTickCount();
	bool completedEntrance = false;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _boardingZoombinis[i];
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

void ShelterZombiniville::buildBoardingZoombiniDrawOrder(Common::Array<uint> &order, ZoombiniRunner *&draggedZoombini) const {
	order.clear();
	draggedZoombini = nullptr;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i]->_dragging) {
			draggedZoombini = _boardingZoombinis[i];
			continue;
		}
		order.push_back(i);
	}
	for (uint i = 0; i < order.size(); i++) {
		for (uint j = i + 1; j < order.size(); j++) {
			if (_boardingZoombinis[order[j]]->_screenPos.y < _boardingZoombinis[order[i]]->_screenPos.y)
				SWAP(order[i], order[j]);
		}
	}
}

void ShelterZombiniville::drawBoardingZoombinis(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	ZoombiniRunner *draggedZoombini = nullptr;
	buildBoardingZoombiniDrawOrder(order, draggedZoombini);
	for (uint i = 0; i < order.size(); i++)
		_vm->_gfx->drawZoombiniRunner(screen, _boardingZoombinis[order[i]]);
	if (draggedZoombini)
		_vm->_gfx->drawZoombiniRunner(screen, draggedZoombini);
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
	ZoombiniRunner *draggedZoombini = nullptr;
	buildBoardingZoombiniDrawOrder(order, draggedZoombini);
	for (uint i = 0; i < order.size(); i++) {
		ZoombiniRunner *zoombini = _boardingZoombinis[order[i]];
		if (zoombini->_hidden)
			continue;
		zoombini->tryStartIdleAnimation(_idleZombAnimation, *_vm->_rnd, tick);
		zoombini->advanceAnimationAfterDraw();
	}
	if (draggedZoombini && !draggedZoombini->_hidden)
		draggedZoombini->advanceAnimationAfterDraw();
}

void ShelterZombiniville::onRenderForeground(ManagedSurface32 *screen) {
	int selectedValues[ZmbTrait::kTraitCount];
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++)
		selectedValues[feature] = _stations[feature].selectedValue;
	_vm->_gfx->drawZoombiniPreview(screen, _bigZombAnimation, selectedValues, Common::Point32(300, 110));
}

void ShelterZombiniville::drawHoverRunners(ManagedSurface32 *screen) {
	const uint32 tick = _vm->getGameTickCount();
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++)
			_vm->_gfx->drawAndUpdateAnimationRunner(screen, _featureButtonRunners[feature][value], tick, 0, ManagedSurface32::kScreenSize.width);
	}
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _quickFillButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _batchFillButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _createButtonRunner, tick, 0, ManagedSurface32::kScreenSize.width);
}

void ShelterZombiniville::updateHoverRunners() {
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++)
			_featureButtonRunners[feature][value]->setHitTestEnabled(_featureCounts[feature][value + 1] < 5);
	}
	// Quick Fill and Batch Fill keep the enabled flag from setup, mirroring
	// the original, which never refreshes them after SetRect.
	_createButtonRunner->setHitTestEnabled(canCreateSelectedZoombini());

	if (!_vm->_gfx->getPageLayerStack()->isScrollLocked() || getDraggedZoombini())
		return;

	const Common::Point32 mousePos = _vm->getMousePos();
	const uint32 tick = _vm->getGameTickCount();
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			AnimationRunner *runner = _featureButtonRunners[feature][value];
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

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			if (_featureButtonRunners[feature][value]->containsHitPoint(Common::Point32(pos))) {
				if (hasActiveEntrance() || 5 <= _featureCounts[feature][value + 1])
					return EventHandleResult::kConsumed;
				_stations[feature].selectedValue = value + 1;
				_vm->_selectedFeatures[feature] = value + 1;
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
		return EventHandleResult::kConsumed;
	}

	if (_quickFillButtonRunner->containsHitPoint(Common::Point32(pos))) {
		_quickFillButtonRunner->start(_vm->getGameTickCount());
		if (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
			if (0 <= _sndQuickFill)
				_vm->getSoundManager()->playWithVolume(_sndQuickFill, _vm->getSoundManager()->_volumeSFX);
			if (_vm->getGameState()->hasReachedZoombiniRegistrationLimit()) {
				return EventHandleResult::kConsumed;
			}
			do {
				randomizeSelectedFeatures();
			} while (!createZoombini(false));
		}
		return EventHandleResult::kConsumed;
	}

	if (_batchFillButtonRunner->containsHitPoint(Common::Point32(pos))) {
		_batchFillButtonRunner->start(_vm->getGameTickCount());
		if (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
			if (0 <= _sndBatchFill)
				_vm->getSoundManager()->playWithVolume(_sndBatchFill, _vm->getSoundManager()->_volumeSFX);
			randomizeSelectedFeatures();
			while (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize) {
				if (_vm->getGameState()->hasReachedZoombiniRegistrationLimit())
					break;
				do {
					randomizeSelectedFeatures();
				} while (!createZoombini(true));
			}
			if (!_boardingZoombinis.empty()) {
				const ZoombiniRunner *last = _boardingZoombinis.back();
				for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
					const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(feature);
					const byte value = last->_traits.getValue(traitIndex);
					_stations[feature].selectedValue = value;
					_vm->_selectedFeatures[feature] = value;
				}
			}
		}
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterZombiniville::onLButtonUp(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_boardingZoombinis, Common::Point32(pos.x, pos.y), true,
																		  _pickupZombAnimation, _vm->getGameTickCount(), nullptr, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult ShelterZombiniville::onMouseMove(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_boardingZoombinis, Common::Point32(pos.x, pos.y), false,
																		  _pickupZombAnimation, _vm->getGameTickCount(), nullptr, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
