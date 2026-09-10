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
#include "common/random.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/shelter_zombiniville.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

ShelterZombiniville::ShelterZombiniville(Zoombini2Engine *vm)
	: ShelterBase(vm), _background(nullptr), _quickFillButton(nullptr), _batchFillButton(nullptr), _goButton(nullptr), _bigZombAnimation(nullptr),
	  _littleZombAnimation(nullptr), _pickupZombAnimation(nullptr), _idleZombAnimation(nullptr), _nameFont(nullptr), _musicId(-1), _sndFeatureSelect(-1),
	  _sndQuickFill(-1), _sndBatchFill(-1),
	  _sndValidZoombini(-1), _sndWrongZoombini(-1) {
	_pageId = kPageZombiniville;
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			_stations[feature].buttonRects[value] = Common::Rect32();
			_stations[feature].drawPos[value] = Common::Point32();
			_featureButtons[feature][value] = nullptr;
		}
		_stations[feature].selectedValue = 0;
	}
}

ShelterZombiniville::~ShelterZombiniville() {
	if (_vm->isReturningToMap())
		GameState::transferSavedRoster(_vm->_globalZoombinis, _vm->getGameState()->_savedRoster);
	delete _background;
	delete _quickFillButton;
	delete _batchFillButton;
	delete _goButton;
	delete _nameFont;
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++)
			delete _featureButtons[feature][value];
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
	_goRect = Common::Rect32(404, 438, 491, 500);
}

Common::Point32 ShelterZombiniville::getSlotPosition(uint index) {
	static const Common::Point32 kSlotPos[kMaxPackSize] = {
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
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		const Common::Point32 slotPos = getSlotPosition(i);
		zoombini->setPosition(slotPos);
		zoombini->_inputEnabled = true;
		zoombini->_placementIndex = i;
		_boardingZoombinis.push_back(zoombini);
	}
	refreshFeatureCounts();
	gameState->registerPageVisit(kPageZombiniville);

	_background = new BitBlock();
	if (!_background->load(Common::Path("#bmp/zombiniville/zoombiniville")))
		warning("ShelterZombiniville: Failed to load background");

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			const Common::String path = Common::String::format("bmp/zombiniville/pikaroll/z1pi%d%d.an", feature + 1, value + 1);
			_featureButtons[feature][value] = new Animation();
			if (!_featureButtons[feature][value]->loadFromFile(Common::Path(path)))
				warning("ShelterZombiniville: Failed to load '%s'", path.c_str());
		}
	}

	_quickFillButton = new Animation();
	_quickFillButton->loadFromFile(Common::Path("bmp/zombiniville/BUMPER-1.AN"));
	_batchFillButton = new Animation();
	_batchFillButton->loadFromFile(Common::Path("bmp/zombiniville/bumper-16.an"));
	_goButton = new Animation();
	_goButton->loadFromFile(Common::Path("bmp/zombiniville/bumper-Valid.an"));

	_bigZombAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombiniville/BigZomb/BigZomb.anm"));
	if (!_bigZombAnimation)
		warning("ShelterZombiniville: Failed to load BigZomb.anm");
	_littleZombAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/littleZomb.anm"));
	if (!_littleZombAnimation)
		warning("ShelterZombiniville: Failed to load littleZomb.anm");
	_pickupZombAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/pris/pris.anm"));
	if (!_pickupZombAnimation)
		warning("ShelterZombiniville: Failed to load pris.anm");
	_idleZombAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/attente2/attenteZomb2.anm"));
	if (!_idleZombAnimation)
		warning("ShelterZombiniville: Failed to load attenteZomb2.anm");
	for (uint i = 0; i < _boardingZoombinis.size(); i++)
		_boardingZoombinis[i]->setDefaultAnimation(_littleZombAnimation, 66);

	_nameFont = new BitmapFont();
	if (!_nameFont->load(Common::Path("bmp/typo"), 16, 16, 16))
		warning("ShelterZombiniville: Failed to load name font");

	setupFeatureRects();
	resetSelectedFeatures();
	_currentName.clear();

	SoundManager *sound = _vm->getSoundManager();
	_musicId = sound->load(true, Common::Path("#sounds/music/ZMR-PickerScreen.wav"), true);
	if (0 <= _musicId)
		sound->playLoop(_musicId);
	_sndFeatureSelect = sound->load(false, Common::Path("sounds/fx/Z-BS11.wav"), false);
	_sndQuickFill = sound->load(false, Common::Path("sounds/fx/Z-BS12.wav"), false);
	_sndBatchFill = sound->load(false, Common::Path("sounds/fx/Z-BS13.wav"), false);
	_sndValidZoombini = sound->load(false, Common::Path("sounds/fx/Z-BS14.wav"), false);
	_sndWrongZoombini = sound->load(false, Common::Path("sounds/fx/WrongZ.wav"), false);
}

void ShelterZombiniville::refreshFeatureCounts() {
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _boardingZoombinis[i];
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
	Common::RandomSource *randomSrc = _vm->getRandom();
	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		const int value = randomSrc->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1;
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
		const ZoombiniState *zoombini = _boardingZoombinis[i];
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
	static const char *const kVowelPairs[30] = {
		"a ", "a ", "a ", "e ", "e ", "e ", "e ", "i ", "i ", "i ", "o ", "o ", "o ", "u ", "u ",
		"y ", "ee", "oo", "yo", "ya", "ye", "ei", "ie", "ai", "ia", "au", "ua", "uo", "ou", "ae"};
	static const char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwx";
	static const char kEndings[] = "aeiou";
	static const char *const kConsonantPairs[39] = {
		"bl", "br", "ch", "cl", "cr", "dr", "dw", "fl", "fr", "gh", "gl", "gr", "kl", "kn", "kr", "kw", "ld", "mp", "nd", "nh",
		"nn", "ph", "pl", "pr", "qu", "qu", "rh", "rn", "sc", "sl", "sm", "sn", "sp", "sr", "st", "sw", "th", "tr", "tw"};

	Common::RandomSource *randomSrc = _vm->getRandom();
	char name[8] = {};
	const int targetLength = randomSrc->getRandomNumber(1) + 4;
	bool useVowelPair = randomSrc->getRandomNumber(98) + 1 < 40;
	int length = 0;
	while (length < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[randomSrc->getRandomNumber(29)];
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
			if (1 < length || randomSrc->getRandomNumber(98) + 1 <= 33) {
				const char *pair = kConsonantPairs[randomSrc->getRandomNumber(38)];
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
				usedConsonantPair = true;
			} else {
				name[length] = kSingleConsonants[randomSrc->getRandomNumber(30)];
				length += 1;
			}
		}
		if (usedConsonantPair && targetLength <= length)
			name[length - 1] = kEndings[randomSrc->getRandomNumber(4)];
		if (length == 2 && name[0] == name[1])
			length = 1;
	}
	return Common::String(name);
}

PathObject *ShelterZombiniville::createEntrancePath(const Common::Point32 &dest) const {
	PathObject *path = new PathObject();
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
	ZoombiniState *zoombini = new ZoombiniState();
	zoombini->setTraits(traits);
	_currentName = generateName();
	Common::strlcpy(zoombini->_name, _currentName.c_str(), sizeof(zoombini->_name));
	zoombini->setPosition(dest);
	zoombini->setDefaultAnimation(_littleZombAnimation, 66);
	zoombini->_inputEnabled = false;
	zoombini->_placementIndex = _boardingZoombinis.size();
	zoombini->_tracksMovementDirection = true;
	const uint32 tick = _vm->getGameTickCount();
	zoombini->startAnimation(nullptr, 66, tick, 50, true);
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
		ZoombiniState *zoombini = _boardingZoombinis[i];
		if (zoombini->_movementPath && !zoombini->advanceMovement(tick)) {
			zoombini->_inputEnabled = true;
			zoombini->resetAnimation();
			completedEntrance = true;
		}
		zoombini->tryStartIdleAnimation(_idleZombAnimation, *_vm->getRandom(), tick, 50);
		zoombini->updateAnimation(tick);
	}
	if (completedEntrance)
		resetSelectedFeatures();
}

void ShelterZombiniville::drawAnimFrame(ManagedSurface32 *screen, const Animation *animation, int frameIndex, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) {
	if (!animation || animation->getFrameCount() == 0)
		return;
	frameIndex = MIN(frameIndex, animation->getFrameCount() - 1);
	const RleBlock *frame = animation->getFrame(frameIndex);
	if (frame)
		frame->drawToScreen(screen, pos, alphaLUT);
}

void ShelterZombiniville::drawBoardingZoombinis(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	ZoombiniState *draggedZoombini = nullptr;
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
	for (uint i = 0; i < order.size(); i++)
		_boardingZoombinis[order[i]]->draw(screen, _vm->getAlphaLUT());
	if (draggedZoombini)
		draggedZoombini->draw(screen, _vm->getAlphaLUT());
}

ZoombiniState *ShelterZombiniville::getDraggedZoombini() const {
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		if (_boardingZoombinis[i]->_dragging)
			return _boardingZoombinis[i];
	}
	return nullptr;
}

void ShelterZombiniville::onRenderScene(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			const Common::Point32 &pos = _stations[feature].drawPos[value];
			drawAnimFrame(screen, _featureButtons[feature][value], 0, Common::Point32(pos.x, pos.y), alphaLUT);
		}
	}

	if (hasActiveEntrance() && _nameFont && _nameFont->isLoaded()) {
		const int width = _nameFont->getStringWidth(_currentName);
		_nameFont->drawString(screen, Common::Point32(367 - width / 2, 280), _currentName, alphaLUT);
	}
}

void ShelterZombiniville::onRenderActors(ManagedSurface32 *screen) {
	drawBoardingZoombinis(screen);
}

void ShelterZombiniville::onRenderForeground(ManagedSurface32 *screen) {
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();
	if (_bigZombAnimation) {
		static constexpr int kBaseCell = 990;
		static constexpr int kFeatureCellBases[ZmbTrait::kTraitCount] = {996, 1002, 1008, 1014};
		static constexpr int kFeatureDrawOrder[ZmbTrait::kTraitCount] = {0, 2, 3, 1};
		const RleBlock *frame = _bigZombAnimation->getFrame(kBaseCell, 0);
		if (frame)
			frame->drawToScreen(screen, Common::Point32(300, 110), alphaLUT);
		for (int i = 0; i < ZmbTrait::kTraitCount; i++) {
			const int feature = kFeatureDrawOrder[i];
			const int value = _stations[feature].selectedValue;
			if (1 <= value && value <= ZmbTrait::kTraitValueCount) {
				frame = _bigZombAnimation->getFrame(kFeatureCellBases[feature] + value, 0);
				if (frame)
					frame->drawToScreen(screen, Common::Point32(300, 110), alphaLUT);
			}
		}
	}

	if (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize) {
		drawAnimFrame(screen, _quickFillButton, 1, Common::Point32(131, 394), alphaLUT);
		drawAnimFrame(screen, _batchFillButton, 1, Common::Point32(230, 412), alphaLUT);
	}
	if (canCreateSelectedZoombini())
		drawAnimFrame(screen, _goButton, 1, Common::Point32(395, 429), alphaLUT);
}

EventHandleResult ShelterZombiniville::onLButtonDown(const Common::Point &pos) {
	if (!hasActiveEntrance()) {
		const uint32 tick = _vm->getGameTickCount();
		for (uint i = 0; i < _boardingZoombinis.size(); i++) {
			if (_boardingZoombinis[i]->beginDrag(Common::Point32(pos.x, pos.y), _pickupZombAnimation, tick, 100))
				return EventHandleResult::kConsumed;
		}
	}

	for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			if (_stations[feature].buttonRects[value].contains(pos.x, pos.y)) {
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

	if (_goRect.contains(pos.x, pos.y)) {
		if (!canCreateSelectedZoombini() || !createZoombini(false)) {
			if (0 <= _sndWrongZoombini)
				_vm->getSoundManager()->playWithVolume(_sndWrongZoombini, _vm->getSoundManager()->_volumeSFX);
			return EventHandleResult::kConsumed;
		}
		if (0 <= _sndValidZoombini)
			_vm->getSoundManager()->playWithVolume(_sndValidZoombini, _vm->getSoundManager()->_volumeSFX);
		return EventHandleResult::kConsumed;
	}

	if (_quickFillRect.contains(pos.x, pos.y) && static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
		if (0 <= _sndQuickFill)
			_vm->getSoundManager()->playWithVolume(_sndQuickFill, _vm->getSoundManager()->_volumeSFX);
		if (_vm->getGameState()->hasReachedZoombiniRegistrationLimit()) {
			return EventHandleResult::kConsumed;
		}
		do {
			randomizeSelectedFeatures();
		} while (!createZoombini(false));
		return EventHandleResult::kConsumed;
	}

	if (_batchFillRect.contains(pos.x, pos.y) && static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
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
			const ZoombiniState *last = _boardingZoombinis.back();
			for (int feature = 0; feature < ZmbTrait::kTraitCount; feature++) {
				const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(feature);
				const byte value = last->_traits.getValue(traitIndex);
				_stations[feature].selectedValue = value;
				_vm->_selectedFeatures[feature] = value;
			}
		}
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterZombiniville::onLButtonUp(const Common::Point &pos) {
	(void)pos;
	ZoombiniState *zoombini = getDraggedZoombini();
	if (!zoombini)
		return EventHandleResult::kPassthrough;
	zoombini->endDrag();
	return EventHandleResult::kConsumed;
}

EventHandleResult ShelterZombiniville::onMouseMove(const Common::Point &pos) {
	ZoombiniState *zoombini = getDraggedZoombini();
	if (!zoombini)
		return EventHandleResult::kPassthrough;
	zoombini->updateDrag(Common::Point32(pos.x, pos.y));
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
