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

#include "common/debug.h"
#include "common/random.h"
#include "common/util.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_crazyturtle.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const PuzzleCrazyTurtle::TurtlePlacement PuzzleCrazyTurtle::kTurtlePlacements[kTurtleCount] = {
	{3, Common::Point32(0, 322)},
	{3, Common::Point32(8, 383)},
	{3, Common::Point32(29, 448)},
	{4, Common::Point32(104, 482)},
	{4, Common::Point32(178, 463)},
	{2, Common::Point32(264, 444)},
	{1, Common::Point32(245, 378)},
	{3, Common::Point32(233, 308)},
	{4, Common::Point32(248, 238)},
	{4, Common::Point32(325, 196)},
	{2, Common::Point32(412, 175)},
	{1, Common::Point32(492, 165)},
	{3, Common::Point32(576, 172)},
	{2, Common::Point32(548, 245)},
	{2, Common::Point32(461, 301)},
	{3, Common::Point32(529, 346)},
};

const Common::Point32 PuzzleCrazyTurtle::kZoombiniPos[kZoombiniCount] = {
	Common::Point32(247, 120),
	Common::Point32(245, 74),
	Common::Point32(202, 117),
	Common::Point32(200, 64),
	Common::Point32(166, 42),
	Common::Point32(166, 87),
	Common::Point32(146, 131),
	Common::Point32(99, 120),
	Common::Point32(131, 75),
	Common::Point32(128, 30),
	Common::Point32(86, 66),
	Common::Point32(62, 105),
	Common::Point32(49, 48),
	Common::Point32(18, 135),
	Common::Point32(25, 91),
	Common::Point32(8, 55),
};

const Common::Point32 PuzzleCrazyTurtle::kPrimaryIconPos[kRuleSlotCount] = {
	Common::Point32(365, 15),
	Common::Point32(395, 22),
	Common::Point32(423, 28),
	Common::Point32(450, 35),
	Common::Point32(477, 42),
};

const Common::Point32 PuzzleCrazyTurtle::kSecondaryIconPos[kRuleSlotCount] = {
	Common::Point32(365, 40),
	Common::Point32(395, 47),
	Common::Point32(423, 54),
	Common::Point32(450, 60),
	Common::Point32(477, 67),
};

PuzzleCrazyTurtle::PuzzleCrazyTurtle(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageCrazyTurtle), _level(1), _primaryFeature(0), _secondaryFeature(1), _remainingMistakes(0), _initialMistakes(0),
	  _motherAnimation(nullptr), _motherSpeechAnimation(nullptr), _smokeAnimation(nullptr), _motherStartImage(nullptr), _motherEndImage(nullptr),
	  _bridgeImage(nullptr), _collapsedBridgeImage(nullptr), _beamImage(nullptr), _musicId(-1) {
	memset(_ruleValues, 0, sizeof(_ruleValues));
	memset(_primaryRuleActive, 0, sizeof(_primaryRuleActive));
	memset(_secondaryRuleActive, 0, sizeof(_secondaryRuleActive));
	for (int type = 0; type < kFeatureCount; type++) {
		_turtleIdleAnimations[type] = nullptr;
		_turtleSpinAnimations[type] = nullptr;
		_turtleFixedImages[type] = nullptr;
		for (int value = 0; value < kFeatureValueCount; value++)
			_traitImages[type][value] = nullptr;
	}
}

PuzzleCrazyTurtle::~PuzzleCrazyTurtle() {
	if (0 <= _musicId) {
		SoundManager *soundManager = _vm->getSoundManager();
		soundManager->stop(_musicId);
		soundManager->unload(_musicId);
	}
	for (int type = 0; type < kFeatureCount; type++) {
		delete _turtleIdleAnimations[type];
		delete _turtleSpinAnimations[type];
		delete _turtleFixedImages[type];
		for (int value = 0; value < kFeatureValueCount; value++)
			delete _traitImages[type][value];
	}
	delete _motherAnimation;
	delete _motherSpeechAnimation;
	delete _smokeAnimation;
	delete _motherStartImage;
	delete _motherEndImage;
	delete _bridgeImage;
	delete _collapsedBridgeImage;
	delete _beamImage;
}

bool PuzzleCrazyTurtle::loadAnimationResource(Animation *&resource, const Common::Path &path) {
	resource = new Animation();
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load animation '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

bool PuzzleCrazyTurtle::loadRleResource(RleBlock *&resource, const Common::Path &path) {
	resource = new RleBlock();
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load RLE graphic '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

void PuzzleCrazyTurtle::init() {
	PuzzleBase::init();

	GameState *gameState = _vm->getGameState();
	_level = CLIP(gameState ? gameState->_level : 1, 1, 4);
	generateRules();
	loadResources();
	placeZoombinis();

	if (SoundManager *soundManager = _vm->getSoundManager()) {
		_musicId = soundManager->load(true, Common::Path("#sounds/music/08-BS01.wav"), true);
		if (0 <= _musicId) {
			soundManager->playLoop(_musicId);
			soundManager->setVolume(_musicId, soundManager->_volumeMusic);
		}
	}

	if (gameState && _vm->_isSavedGame)
		gameState->registerPageVisit(kPageCrazyTurtle, 1);

	debug(1, "CrazyTurtlePuzzle::init: level=%d primary=%d secondary=%d mistakes=%d party=%u", _level, _primaryFeature,
		  _secondaryFeature, _remainingMistakes, _puzzleZoombinis.size());
}

void PuzzleCrazyTurtle::loadResources() {
	for (int type = 0; type < kFeatureCount; type++) {
		const int resourceNumber = type + 1;
		loadAnimationResource(_turtleIdleAnimations[type],
							  Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/ATTENTE/%d/%d.AN", resourceNumber, resourceNumber)));
		loadAnimationResource(_turtleSpinAnimations[type],
							  Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/tourbillonne/%d/%d.AN", resourceNumber, resourceNumber)));
		loadRleResource(_turtleFixedImages[type],
						Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/tourbillonne/%d/FIXE%d.RB", resourceNumber, resourceNumber)));
	}

	loadAnimationResource(_motherAnimation, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/mere.an"));
	loadAnimationResource(_motherSpeechAnimation, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/PARLE/PARLE.AN"));
	loadAnimationResource(_smokeAnimation, Common::Path("Bmp/crazy_turtle/smokey.an"));
	loadRleResource(_motherStartImage, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/meredebut.rb"));
	loadRleResource(_motherEndImage, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/MEREFIN.RB"));
	loadRleResource(_bridgeImage, Common::Path("Bmp/crazy_turtle/PONT.RB"));
	loadRleResource(_collapsedBridgeImage, Common::Path("Bmp/crazy_turtle/pontKC.rb"));
	loadRleResource(_beamImage, Common::Path("Bmp/crazy_turtle/poutrelle.rb"));

	for (int feature = 0; feature < kFeatureCount; feature++) {
		for (int value = 0; value < kFeatureValueCount; value++) {
			loadRleResource(_traitImages[feature][value],
							Common::Path(Common::String::format("Bmp/mystic_marsh/TRAITS/%d-%d.RB", feature + 1, value + 1)));
		}
	}
}

void PuzzleCrazyTurtle::generateRules() {
	Common::RandomSource *randomSrc = _vm->getRandom();
	_primaryFeature = randomSrc->getRandomNumber(kFeatureCount - 1);
	int remainingFeatures[kFeatureCount - 1];
	int remainingCount = 0;
	for (int feature = 0; feature < kFeatureCount; feature++) {
		if (feature != _primaryFeature) {
			remainingFeatures[remainingCount] = feature;
			remainingCount += 1;
		}
	}
	_secondaryFeature = remainingFeatures[randomSrc->getRandomNumber(remainingCount - 1)];

	for (int group = 0; group < kRuleGroupCount; group++)
		generateFeatureOrder(group);
	memset(_primaryRuleActive, 0, sizeof(_primaryRuleActive));
	memset(_secondaryRuleActive, 0, sizeof(_secondaryRuleActive));

	switch (_level) {
	case 1:
		_remainingMistakes = 3;
		for (int slot = 0; slot < kRuleSlotCount; slot++)
			_primaryRuleActive[slot] = true;
		break;
	case 2: {
		const int visibleRuleCount = randomSrc->getRandomNumber(1) + 1;
		_remainingMistakes = visibleRuleCount == 1 ? 5 : 4;
		activateRandomRuleSlots(_primaryRuleActive, visibleRuleCount);
		break;
	}
	case 3:
		_remainingMistakes = 6;
		activateRandomRuleSlots(_primaryRuleActive, 1);
		activateRandomRuleSlots(_secondaryRuleActive, 1);
		break;
	case 4:
		_remainingMistakes = 7;
		break;
	default:
		break;
	}
	_initialMistakes = _remainingMistakes;
}

void PuzzleCrazyTurtle::generateFeatureOrder(int group) {
	int values[kFeatureValueCount] = {1, 2, 3, 4, 5};
	int remainingCount = kFeatureValueCount;
	Common::RandomSource *randomSrc = _vm->getRandom();
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		const int selected = randomSrc->getRandomNumber(remainingCount - 1);
		_ruleValues[group][slot] = values[selected];
		for (int value = selected; value + 1 < remainingCount; value++)
			values[value] = values[value + 1];
		remainingCount -= 1;
	}
}

void PuzzleCrazyTurtle::activateRandomRuleSlots(bool *slots, int count) {
	int available[kRuleSlotCount] = {0, 1, 2, 3, 4};
	int availableCount = kRuleSlotCount;
	Common::RandomSource *randomSrc = _vm->getRandom();
	for (int selectedCount = 0; selectedCount < count; selectedCount++) {
		const int selected = randomSrc->getRandomNumber(availableCount - 1);
		slots[available[selected]] = true;
		for (int slot = selected; slot + 1 < availableCount; slot++)
			available[slot] = available[slot + 1];
		availableCount -= 1;
	}
}

void PuzzleCrazyTurtle::placeZoombinis() {
	const uint count = MIN(static_cast<uint>(kZoombiniCount), _puzzleZoombinis.size());
	for (uint index = 0; index < count; index++) {
		ZoombiniState *zoombini = _puzzleZoombinis[index];
		zoombini->setPosition(kZoombiniPos[index]);
		zoombini->setDefaultAnimation(_zoombiniAnimation, 66);
		zoombini->_inputEnabled = true;
		zoombini->_puzzleStatus = 0;
	}
}

void PuzzleCrazyTurtle::onUpdate() {
	// The initial puzzle state remains stable until the player chooses a
	// Zoombini placement or uses the three-button controls to leave the current game.
}

void PuzzleCrazyTurtle::onRenderBackground(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));
}

void PuzzleCrazyTurtle::onRenderScene(ManagedSurface32 *screen) {
	drawBridgeState(screen);
	drawMother(screen);
	drawTurtles(screen);
	drawRuleHints(screen);
}

void PuzzleCrazyTurtle::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleCrazyTurtle::drawBridgeState(ManagedSurface32 *screen) const {
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();
	if (0 < _remainingMistakes) {
		if (_bridgeImage)
			_bridgeImage->drawToScreen(screen, Common::Point32(10, 220), alphaLUT);
	} else if (_collapsedBridgeImage) {
		_collapsedBridgeImage->drawToScreen(screen, Common::Point32(10, 220), alphaLUT);
	}

	if (!_beamImage)
		return;
	Common::Point32 beamPos(11 * _remainingMistakes + 9, 310 - 12 * _remainingMistakes);
	for (int beam = 0; beam < _remainingMistakes; beam++) {
		_beamImage->drawToScreen(screen, beamPos, alphaLUT);
		beamPos.x -= 11;
		beamPos.y += 12;
	}
}

void PuzzleCrazyTurtle::drawRuleHints(ManagedSurface32 *screen) const {
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		if (!_primaryRuleActive[slot])
			continue;
		const RleBlock *primary = _traitImages[_primaryFeature][_ruleValues[0][slot] - 1];
		if (primary)
			primary->drawToScreen(screen, Common::Point32(kPrimaryIconPos[slot].x - 5, kPrimaryIconPos[slot].y), alphaLUT);
		if (3 <= _level) {
			const RleBlock *secondary = _traitImages[_secondaryFeature][_ruleValues[1][slot] - 1];
			if (secondary)
				secondary->drawToScreen(screen, Common::Point32(kSecondaryIconPos[slot].x - 5, kSecondaryIconPos[slot].y), alphaLUT);
		}
	}
}

void PuzzleCrazyTurtle::drawTurtles(ManagedSurface32 *screen) const {
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();
	for (int index = 0; index < kTurtleCount; index++) {
		const TurtlePlacement &placement = kTurtlePlacements[index];
		const RleBlock *image = _turtleFixedImages[placement.type - 1];
		if (image)
			image->drawToScreen(screen, placement.pos, alphaLUT);
	}
}

void PuzzleCrazyTurtle::drawMother(ManagedSurface32 *screen) const {
	if (_motherStartImage)
		_motherStartImage->drawToScreen(screen, Common::Point32(488, 310), _vm->getAlphaLUT());
}

EventHandleResult PuzzleCrazyTurtle::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	// Three-button navigation is handled before page input. Turtle placement remains
	// unchanged here until the path-driven Zoombini interaction is available.
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
