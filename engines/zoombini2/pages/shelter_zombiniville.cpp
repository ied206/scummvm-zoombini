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
#include "zoombini2/path.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/ui.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

Zombiniville::Zombiniville(Zoombini2Engine *engine)
	: ShelterPage(engine), _background(nullptr), _quickFillButton(nullptr), _batchFillButton(nullptr), _goButton(nullptr), _bigZombGfx(nullptr),
	  _littleZombGfx(nullptr), _nameFont(nullptr), _musicId(-1), _sndFeatureSelect(-1), _sndQuickFill(-1), _sndBatchFill(-1),
	  _sndValidZoombini(-1), _sndWrongZoombini(-1) {
	_pageId = kPageZombiniville;
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 0; value < kNumFeatureValues; value++) {
			_stations[feature].buttonRects[value] = Common::Rect();
			_stations[feature].drawPos[value] = Common::Point();
			_featureButtons[feature][value] = nullptr;
		}
		_stations[feature].selectedValue = 0;
	}
}

Zombiniville::~Zombiniville() {
	if (_engine->isReturningToMap())
		GameState::transferSavedRoster(_engine->_globalZoombinis, _engine->getGameState()->_zoombinis);
	for (uint i = 0; i < _entrancePaths.size(); i++)
		delete _entrancePaths[i];
	delete _background;
	delete _quickFillButton;
	delete _batchFillButton;
	delete _goButton;
	delete _bigZombGfx;
	delete _littleZombGfx;
	delete _nameFont;
	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 0; value < kNumFeatureValues; value++)
			delete _featureButtons[feature][value];
	}

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		const int sounds[] = {_musicId, _sndFeatureSelect, _sndQuickFill, _sndBatchFill, _sndValidZoombini, _sndWrongZoombini};
		for (uint i = 0; i < ARRAYSIZE(sounds); i++) {
			if (0 <= sounds[i])
				sound->unload(sounds[i]);
		}
	}
}

void Zombiniville::setupFeatureRects() {
	// Bottom station: feet.
	_stations[0].drawPos[0] = Common::Point(237, 345);
	_stations[0].drawPos[1] = Common::Point(292, 351);
	_stations[0].drawPos[2] = Common::Point(398, 355);
	_stations[0].drawPos[3] = Common::Point(346, 349);
	_stations[0].drawPos[4] = Common::Point(438, 369);
	_stations[0].buttonRects[0] = Common::Rect(238, 349, 286, 390);
	_stations[0].buttonRects[1] = Common::Rect(291, 352, 344, 396);
	_stations[0].buttonRects[2] = Common::Rect(398, 359, 433, 403);
	_stations[0].buttonRects[3] = Common::Rect(349, 353, 394, 398);
	_stations[0].buttonRects[4] = Common::Rect(438, 365, 478, 406);

	// Right station: noses.
	_stations[1].drawPos[0] = Common::Point(534, 126);
	_stations[1].drawPos[1] = Common::Point(529, 167);
	_stations[1].drawPos[2] = Common::Point(526, 214);
	_stations[1].drawPos[3] = Common::Point(522, 255);
	_stations[1].drawPos[4] = Common::Point(518, 296);
	_stations[1].buttonRects[0] = Common::Rect(534, 118, 570, 151);
	_stations[1].buttonRects[1] = Common::Rect(529, 159, 564, 197);
	_stations[1].buttonRects[2] = Common::Rect(522, 207, 561, 247);
	_stations[1].buttonRects[3] = Common::Rect(518, 255, 557, 298);
	_stations[1].buttonRects[4] = Common::Rect(511, 303, 552, 341);

	// Top station: hair.
	_stations[2].drawPos[0] = Common::Point(229, 41);
	_stations[2].drawPos[1] = Common::Point(286, 45);
	_stations[2].drawPos[2] = Common::Point(404, 46);
	_stations[2].drawPos[3] = Common::Point(345, 44);
	_stations[2].drawPos[4] = Common::Point(453, 45);
	_stations[2].buttonRects[0] = Common::Rect(226, 40, 276, 85);
	_stations[2].buttonRects[1] = Common::Rect(284, 42, 335, 87);
	_stations[2].buttonRects[2] = Common::Rect(403, 44, 448, 85);
	_stations[2].buttonRects[3] = Common::Rect(345, 43, 396, 87);
	_stations[2].buttonRects[4] = Common::Rect(454, 44, 501, 85);

	// Left station: eyes.
	_stations[3].drawPos[0] = Common::Point(162, 155);
	_stations[3].drawPos[1] = Common::Point(165, 109);
	_stations[3].drawPos[2] = Common::Point(163, 195);
	_stations[3].drawPos[3] = Common::Point(157, 233);
	_stations[3].drawPos[4] = Common::Point(157, 275);
	_stations[3].buttonRects[0] = Common::Rect(164, 153, 207, 186);
	_stations[3].buttonRects[1] = Common::Rect(162, 112, 203, 147);
	_stations[3].buttonRects[2] = Common::Rect(166, 190, 209, 223);
	_stations[3].buttonRects[3] = Common::Rect(166, 228, 212, 262);
	_stations[3].buttonRects[4] = Common::Rect(167, 271, 213, 304);

	_quickFillRect = Common::Rect(135, 399, 211, 448);
	_batchFillRect = Common::Rect(239, 416, 318, 477);
	_goRect = Common::Rect(404, 438, 491, 500);
}

Common::Point Zombiniville::getSlotPosition(uint index) {
	static const Common::Point kSlotPositions[kMaxPackSize] = {
		Common::Point(490, 524), Common::Point(500, 485), Common::Point(452, 526), Common::Point(455, 485),
		Common::Point(410, 527), Common::Point(408, 484), Common::Point(370, 527), Common::Point(366, 485),
		Common::Point(327, 528), Common::Point(327, 482), Common::Point(286, 528), Common::Point(285, 482),
		Common::Point(246, 528), Common::Point(248, 483), Common::Point(209, 488), Common::Point(177, 465)};
	return index < static_cast<uint>(kMaxPackSize) ? kSlotPositions[index] : kSlotPositions[kMaxPackSize - 1];
}

void Zombiniville::init() {
	debug(1, "Zombiniville::init");
	_engine->clearGlobalZoombinis();
	_boardingZoombinis.clear();
	_entrancePaths.clear();
	GameState *gameState = _engine->getGameState();
	if (_engine->_isSavedGame)
		GameState::transferSavedRoster(gameState->_zoombinis, _engine->_globalZoombinis);
	for (uint i = 0; i < _engine->_globalZoombinis.size(); i++) {
		ZoombiniState *zoombini = _engine->_globalZoombinis[i];
		const Common::Point slot = getSlotPosition(i);
		zoombini->_position = Common::Point32(slot);
		zoombini->_targetPosition = zoombini->_position;
		zoombini->_activeFlag = 1;
		zoombini->_zoombiniIndex = 33;
		_boardingZoombinis.push_back(zoombini);
		_entrancePaths.push_back(nullptr);
	}
	refreshFeatureCounts();
	gameState->registerWorldVisit(kPageZombiniville);

	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/zombiniville/zoombiniville")))
		warning("Zombiniville: Failed to load background");

	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 0; value < kNumFeatureValues; value++) {
			const Common::String path = Common::String::format("bmp/zombiniville/pikaroll/z1pi%d%d.an", feature + 1, value + 1);
			_featureButtons[feature][value] = new Animation();
			if (!_featureButtons[feature][value]->loadFromFile(Common::Path(path)))
				warning("Zombiniville: Failed to load '%s'", path.c_str());
		}
	}

	_quickFillButton = new Animation();
	_quickFillButton->loadFromFile(Common::Path("bmp/zombiniville/BUMPER-1.AN"));
	_batchFillButton = new Animation();
	_batchFillButton->loadFromFile(Common::Path("bmp/zombiniville/bumper-16.an"));
	_goButton = new Animation();
	_goButton->loadFromFile(Common::Path("bmp/zombiniville/bumper-Valid.an"));

	_bigZombGfx = new ZoombiniGraphics();
	if (!_bigZombGfx->loadFromFile(Common::Path("bmp/zombiniville/BigZomb/BigZomb.anm")))
		warning("Zombiniville: Failed to load BigZomb.anm");
	_littleZombGfx = new ZoombiniGraphics();
	if (!_littleZombGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm")))
		warning("Zombiniville: Failed to load littleZomb.anm");

	_nameFont = new BitmapFont();
	if (!_nameFont->load(Common::Path("bmp/typo"), 16, 16, 16))
		warning("Zombiniville: Failed to load name font");

	setupFeatureRects();
	resetSelectedFeatures();
	_currentName.clear();

	SoundManager *sound = _engine->getSoundManager();
	_musicId = sound->load(true, Common::Path("sounds/music/ZMR-PickerScreen.wav"), true);
	if (0 <= _musicId)
		sound->playLoop(_musicId);
	_sndFeatureSelect = sound->load(false, Common::Path("sounds/fx/Z-BS11.wav"), false);
	_sndQuickFill = sound->load(false, Common::Path("sounds/fx/Z-BS12.wav"), false);
	_sndBatchFill = sound->load(false, Common::Path("sounds/fx/Z-BS13.wav"), false);
	_sndValidZoombini = sound->load(false, Common::Path("sounds/fx/Z-BS14.wav"), false);
	_sndWrongZoombini = sound->load(false, Common::Path("sounds/fx/WrongZ.wav"), false);
}

void Zombiniville::refreshFeatureCounts() {
	memset(_featureCounts, 0, sizeof(_featureCounts));
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _boardingZoombinis[i];
		const byte features[kNumFeatures] = {zoombini->_featureA, zoombini->_featureB, zoombini->_featureC, zoombini->_featureD};
		for (int feature = 0; feature < kNumFeatures; feature++) {
			if (1 <= features[feature] && features[feature] <= kNumFeatureValues)
				_featureCounts[feature][features[feature]] += 1;
		}
	}
}

void Zombiniville::resetSelectedFeatures() {
	for (int feature = 0; feature < kNumFeatures; feature++) {
		_stations[feature].selectedValue = 0;
		_engine->_selectedFeatures[feature] = -1;
	}
}

void Zombiniville::randomizeSelectedFeatures() {
	Common::RandomSource *randomSource = _engine->getRandom();
	for (int feature = 0; feature < kNumFeatures; feature++) {
		const int value = randomSource->getRandomNumber(kNumFeatureValues - 1) + 1;
		_stations[feature].selectedValue = value;
		_engine->_selectedFeatures[feature] = value;
	}
}

bool Zombiniville::hasActiveEntrance() const {
	for (uint i = 0; i < _entrancePaths.size(); i++) {
		if (_entrancePaths[i])
			return true;
	}
	return false;
}

bool Zombiniville::canUseGoButton() const {
	return static_cast<int>(_boardingZoombinis.size()) == kMaxPackSize;
}

bool Zombiniville::canCreateSelectedZoombini() const {
	if (kMaxPackSize <= static_cast<int>(_boardingZoombinis.size()) || hasActiveEntrance())
		return false;
	for (int feature = 0; feature < kNumFeatures; feature++) {
		if (_stations[feature].selectedValue == 0)
			return false;
	}
	return true;
}

bool Zombiniville::passesPackFeatureLimits(byte featureA, byte featureB, byte featureC, byte featureD) const {
	int counts[kNumFeatures][kNumFeatureValues + 1] = {};
	const byte candidateFeatures[kNumFeatures] = {featureA, featureB, featureC, featureD};
	for (int feature = 0; feature < kNumFeatures; feature++) {
		if (candidateFeatures[feature] < 1 || kNumFeatureValues < candidateFeatures[feature])
			return false;
		counts[feature][candidateFeatures[feature]] += 1;
	}

	const uint16 candidateHash = ZoombiniState::calculateFeatureHash(featureA, featureB, featureC, featureD);
	int matchingCombinations = 0;
	for (uint i = 0; i < _boardingZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _boardingZoombinis[i];
		const byte features[kNumFeatures] = {zoombini->_featureA, zoombini->_featureB, zoombini->_featureC, zoombini->_featureD};
		for (int feature = 0; feature < kNumFeatures; feature++) {
			if (features[feature] < 1 || kNumFeatureValues < features[feature])
				return false;
			counts[feature][features[feature]] += 1;
		}
		if (zoombini->_featureHash == candidateHash)
			matchingCombinations += 1;
	}

	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 1; value <= kNumFeatureValues; value++) {
			if (5 < counts[feature][value])
				return false;
		}
	}
	return matchingCombinations < 2;
}

Common::String Zombiniville::generateName() {
	static const char *const kVowelPairs[30] = {
		"a ", "a ", "a ", "e ", "e ", "e ", "e ", "i ", "i ", "i ", "o ", "o ", "o ", "u ", "u ",
		"y ", "ee", "oo", "yo", "ya", "ye", "ei", "ie", "ai", "ia", "au", "ua", "uo", "ou", "ae"};
	static const char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwx";
	static const char kEndings[] = "aeiou";
	static const char *const kConsonantPairs[39] = {
		"bl", "br", "ch", "cl", "cr", "dr", "dw", "fl", "fr", "gh", "gl", "gr", "kl", "kn", "kr", "kw", "ld", "mp", "nd", "nh",
		"nn", "ph", "pl", "pr", "qu", "qu", "rh", "rn", "sc", "sl", "sm", "sn", "sp", "sr", "st", "sw", "th", "tr", "tw"};

	Common::RandomSource *randomSource = _engine->getRandom();
	char name[8] = {};
	const int targetLength = randomSource->getRandomNumber(1) + 4;
	bool useVowelPair = randomSource->getRandomNumber(98) + 1 < 40;
	int length = 0;
	while (length < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[randomSource->getRandomNumber(29)];
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
			if (1 < length || randomSource->getRandomNumber(98) + 1 <= 33) {
				const char *pair = kConsonantPairs[randomSource->getRandomNumber(38)];
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
				usedConsonantPair = true;
			} else {
				name[length] = kSingleConsonants[randomSource->getRandomNumber(30)];
				length += 1;
			}
		}
		if (usedConsonantPair && targetLength <= length)
			name[length - 1] = kEndings[randomSource->getRandomNumber(4)];
		if (length == 2 && name[0] == name[1])
			length = 1;
	}
	return Common::String(name);
}

PathObject *Zombiniville::createEntrancePath(const Common::Point &destination) const {
	PathObject *path = new PathObject();
	CurveSegment *segment = new CurveSegment();
	segment->init(
		Common::Point32(-70, 400), Common::Point32(destination.x / 3, 420),
		Common::Point32(2 * (destination.x / 3), 450), Common::Point32(destination), 2, 0);
	path->segments.push_back(segment);
	path->endPosition = Common::Point32(destination);
	path->start(_engine->getGameTickCount());
	return path;
}

bool Zombiniville::createZoombini(bool allowConcurrentEntrances) {
	if ((!allowConcurrentEntrances && hasActiveEntrance()) || kMaxPackSize <= static_cast<int>(_boardingZoombinis.size()))
		return false;

	byte features[kNumFeatures];
	for (int feature = 0; feature < kNumFeatures; feature++) {
		if (_stations[feature].selectedValue == 0)
			return false;
		features[feature] = static_cast<byte>(_stations[feature].selectedValue);
	}

	GameState *gameState = _engine->getGameState();
	if (gameState->hasRegisteredEveryFeatureCombination() || !gameState->canRegisterFeatures(features[0], features[1], features[2], features[3]))
		return false;
	if (!gameState->hasRelaxedPackFeatureLimits() && !passesPackFeatureLimits(features[0], features[1], features[2], features[3]))
		return false;
	if (!gameState->registerFeatures(features[0], features[1], features[2], features[3]))
		return false;

	const Common::Point destination = getSlotPosition(_boardingZoombinis.size());
	ZoombiniState *zoombini = new ZoombiniState();
	zoombini->setFeatures(features[0], features[1], features[2], features[3]);
	_currentName = generateName();
	memcpy(zoombini->_extraState, _currentName.c_str(), _currentName.size() + 1);
	zoombini->_position = Common::Point32(-70, 400);
	zoombini->_targetPosition = Common::Point32(destination);
	zoombini->_activeFlag = 0;
	zoombini->_zoombiniIndex = 33;
	_engine->_globalZoombinis.push_back(zoombini);
	_boardingZoombinis.push_back(zoombini);
	_entrancePaths.push_back(createEntrancePath(destination));
	for (int feature = 0; feature < kNumFeatures; feature++)
		_featureCounts[feature][features[feature]] += 1;

	debug(2, "Zombiniville: Created '%s' with features %d/%d/%d/%d in slot %u", _currentName.c_str(), features[0], features[1], features[2],
		  features[3], _boardingZoombinis.size() - 1);
	return true;
}

void Zombiniville::update() {
	const uint32 tick = _engine->getGameTickCount();
	bool completedEntrance = false;
	for (uint i = 0; i < _entrancePaths.size(); i++) {
		PathObject *path = _entrancePaths[i];
		if (!path)
			continue;
		Common::Point32 position;
		if (path->advance(tick, position)) {
			_boardingZoombinis[i]->_position = position;
		} else {
			_boardingZoombinis[i]->_position = path->endPosition;
			_boardingZoombinis[i]->_activeFlag = 1;
			delete path;
			_entrancePaths[i] = nullptr;
			completedEntrance = true;
		}
	}
	if (completedEntrance)
		resetSelectedFeatures();
}

void Zombiniville::drawAnimationFrame(Graphics::ManagedSurface *screen, const Animation *animation, int frameIndex, int x, int y,
									  const byte alphaLUT[256][256]) {
	if (!animation || animation->getFrameCount() == 0)
		return;
	frameIndex = MIN(frameIndex, animation->getFrameCount() - 1);
	const RleBlock *frame = animation->getFrame(frameIndex);
	if (frame)
		frame->drawToScreen(screen, x, y, alphaLUT);
}

void Zombiniville::drawZoombini(Graphics::ManagedSurface *screen, const ZoombiniState &zoombini, int cellIndex) const {
	if (!_littleZombGfx)
		return;
	const byte(*alphaLUT)[256] = _engine->getAlphaLUT();
	const int baseIndex = cellIndex * ZoombiniGraphics::kDim1 * ZoombiniGraphics::kDim2;
	const RleBlock *frame = _littleZombGfx->getFrame(baseIndex, 0);
	if (frame)
		frame->drawToScreen(screen, zoombini._position.x, zoombini._position.y, alphaLUT);
	const byte features[kNumFeatures] = {zoombini._featureA, zoombini._featureB, zoombini._featureC, zoombini._featureD};
	for (int layer = 1; layer <= kNumFeatures; layer++) {
		frame = _littleZombGfx->getFrame(baseIndex + layer * ZoombiniGraphics::kDim2 + features[layer - 1], 0);
		if (frame)
			frame->drawToScreen(screen, zoombini._position.x, zoombini._position.y, alphaLUT);
	}
}

void Zombiniville::drawBoardingZoombinis(Graphics::ManagedSurface *screen) const {
	Common::Array<uint> order;
	for (uint i = 0; i < _boardingZoombinis.size(); i++)
		order.push_back(i);
	for (uint i = 0; i < order.size(); i++) {
		for (uint j = i + 1; j < order.size(); j++) {
			if (_boardingZoombinis[order[j]]->_position.y < _boardingZoombinis[order[i]]->_position.y)
				SWAP(order[i], order[j]);
		}
	}
	for (uint i = 0; i < order.size(); i++)
		drawZoombini(screen, *_boardingZoombinis[order[i]], 66);
}

void Zombiniville::draw(Graphics::ManagedSurface *screen) {
	if (_background)
		_background->drawToSurface(screen, 0, 0);
	const byte(*alphaLUT)[256] = _engine->getAlphaLUT();

	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 0; value < kNumFeatureValues; value++) {
			const Common::Point &position = _stations[feature].drawPos[value];
			drawAnimationFrame(screen, _featureButtons[feature][value], 0, position.x, position.y, alphaLUT);
		}
	}

	if (_bigZombGfx) {
		static constexpr int kBaseCell = 990;
		static constexpr int kFeatureCellBases[kNumFeatures] = {996, 1002, 1008, 1014};
		static constexpr int kFeatureDrawOrder[kNumFeatures] = {0, 2, 3, 1};
		const RleBlock *frame = _bigZombGfx->getFrame(kBaseCell, 0);
		if (frame)
			frame->drawToScreen(screen, 300, 110, alphaLUT);
		for (int i = 0; i < kNumFeatures; i++) {
			const int feature = kFeatureDrawOrder[i];
			const int value = _stations[feature].selectedValue;
			if (1 <= value && value <= kNumFeatureValues) {
				frame = _bigZombGfx->getFrame(kFeatureCellBases[feature] + value, 0);
				if (frame)
					frame->drawToScreen(screen, 300, 110, alphaLUT);
			}
		}
	}

	if (hasActiveEntrance() && _nameFont && _nameFont->isLoaded()) {
		const int width = _nameFont->getStringWidth(_currentName);
		_nameFont->drawString(screen, 367 - width / 2, 280, _currentName, alphaLUT);
	}

	drawBoardingZoombinis(screen);
	if (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize) {
		drawAnimationFrame(screen, _quickFillButton, 1, 131, 394, alphaLUT);
		drawAnimationFrame(screen, _batchFillButton, 1, 230, 412, alphaLUT);
	}
	if (canCreateSelectedZoombini())
		drawAnimationFrame(screen, _goButton, 1, 395, 429, alphaLUT);
}

void Zombiniville::handleClick(const Common::Point &pos) {
	for (int feature = 0; feature < kNumFeatures; feature++) {
		for (int value = 0; value < kNumFeatureValues; value++) {
			if (_stations[feature].buttonRects[value].contains(pos)) {
				if (hasActiveEntrance() || 5 <= _featureCounts[feature][value + 1])
					return;
				_stations[feature].selectedValue = value + 1;
				_engine->_selectedFeatures[feature] = value + 1;
				if (0 <= _sndFeatureSelect)
					_engine->getSoundManager()->playWithVolume(_sndFeatureSelect, _engine->getSoundManager()->_volumeSFX);
				return;
			}
		}
	}

	if (_goRect.contains(pos)) {
		if (!canCreateSelectedZoombini() || !createZoombini(false)) {
			if (0 <= _sndWrongZoombini)
				_engine->getSoundManager()->playWithVolume(_sndWrongZoombini, _engine->getSoundManager()->_volumeSFX);
			return;
		}
		if (0 <= _sndValidZoombini)
			_engine->getSoundManager()->playWithVolume(_sndValidZoombini, _engine->getSoundManager()->_volumeSFX);
		return;
	}

	if (_quickFillRect.contains(pos) && static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
		if (0 <= _sndQuickFill)
			_engine->getSoundManager()->playWithVolume(_sndQuickFill, _engine->getSoundManager()->_volumeSFX);
		if (_engine->getGameState()->hasRegisteredEveryFeatureCombination()) {
			return;
		}
		do {
			randomizeSelectedFeatures();
		} while (!createZoombini(false));
		return;
	}

	if (_batchFillRect.contains(pos) && static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize && !hasActiveEntrance()) {
		if (0 <= _sndBatchFill)
			_engine->getSoundManager()->playWithVolume(_sndBatchFill, _engine->getSoundManager()->_volumeSFX);
		randomizeSelectedFeatures();
		while (static_cast<int>(_boardingZoombinis.size()) < kMaxPackSize) {
			if (_engine->getGameState()->hasRegisteredEveryFeatureCombination())
				break;
			do {
				randomizeSelectedFeatures();
			} while (!createZoombini(true));
		}
		if (!_boardingZoombinis.empty()) {
			const ZoombiniState *last = _boardingZoombinis.back();
			const byte features[kNumFeatures] = {last->_featureA, last->_featureB, last->_featureC, last->_featureD};
			for (int feature = 0; feature < kNumFeatures; feature++) {
				_stations[feature].selectedValue = features[feature];
				_engine->_selectedFeatures[feature] = features[feature];
			}
		}
	}
}

} // End of namespace Zoombini2
