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
#include "common/util.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_crazyturtle.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleCrazyTurtle::kMusicPath;
constexpr const char *PuzzleCrazyTurtle::kTurtleIdleFormat;
constexpr const char *PuzzleCrazyTurtle::kTurtleSpinFormat;
constexpr const char *PuzzleCrazyTurtle::kTurtleFixedFormat;
constexpr const char *PuzzleCrazyTurtle::kMotherPath;
constexpr const char *PuzzleCrazyTurtle::kMotherSpeechPath;
constexpr const char *PuzzleCrazyTurtle::kSmokePath;
constexpr const char *PuzzleCrazyTurtle::kMotherStartPath;
constexpr const char *PuzzleCrazyTurtle::kMotherEndPath;
constexpr const char *PuzzleCrazyTurtle::kBridgePath;
constexpr const char *PuzzleCrazyTurtle::kCollapsedBridgePath;
constexpr const char *PuzzleCrazyTurtle::kBeamPath;
constexpr const char *PuzzleCrazyTurtle::kTraitFormat;

constexpr PuzzleCrazyTurtle::TurtlePlacement PuzzleCrazyTurtle::kTurtlePlacements[kTurtleCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kZoombiniPos[kZoombiniCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kPrimaryIconPos[kRuleSlotCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kSecondaryIconPos[kRuleSlotCount];

PuzzleCrazyTurtle::PuzzleCrazyTurtle(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageCrazyTurtle) {
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
	resource = new Animation(_vm);
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load animation '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

bool PuzzleCrazyTurtle::loadRleResource(RleBlock *&resource, const Common::Path &path) {
	resource = new RleBlock(_vm);
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
		_musicId = soundManager->load(true, Common::Path(kMusicPath), true);
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
							  Common::Path(Common::String::format(kTurtleIdleFormat, resourceNumber, resourceNumber)));
		loadAnimationResource(_turtleSpinAnimations[type],
							  Common::Path(Common::String::format(kTurtleSpinFormat, resourceNumber, resourceNumber)));
		loadRleResource(_turtleFixedImages[type],
						Common::Path(Common::String::format(kTurtleFixedFormat, resourceNumber, resourceNumber)));
	}

	loadAnimationResource(_motherAnimation, Common::Path(kMotherPath));
	loadAnimationResource(_motherSpeechAnimation, Common::Path(kMotherSpeechPath));
	loadAnimationResource(_smokeAnimation, Common::Path(kSmokePath));
	loadRleResource(_motherStartImage, Common::Path(kMotherStartPath));
	loadRleResource(_motherEndImage, Common::Path(kMotherEndPath));
	loadRleResource(_bridgeImage, Common::Path(kBridgePath));
	loadRleResource(_collapsedBridgeImage, Common::Path(kCollapsedBridgePath));
	loadRleResource(_beamImage, Common::Path(kBeamPath));

	for (int feature = 0; feature < kFeatureCount; feature++) {
		for (int value = 0; value < kFeatureValueCount; value++) {
			loadRleResource(_traitImages[feature][value],
							Common::Path(Common::String::format(kTraitFormat, feature + 1, value + 1)));
		}
	}
}

void PuzzleCrazyTurtle::generateRules() {
	_primaryFeature = _vm->_rnd->getRandomNumber(kFeatureCount - 1);
	int remainingFeatures[kFeatureCount - 1];
	int remainingCount = 0;
	for (int feature = 0; feature < kFeatureCount; feature++) {
		if (feature != _primaryFeature) {
			remainingFeatures[remainingCount] = feature;
			remainingCount += 1;
		}
	}
	_secondaryFeature = remainingFeatures[_vm->_rnd->getRandomNumber(remainingCount - 1)];

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
		const int visibleRuleCount = _vm->_rnd->getRandomNumber(1) + 1;
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
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		const int selected = _vm->_rnd->getRandomNumber(remainingCount - 1);
		_ruleValues[group][slot] = values[selected];
		for (int value = selected; value + 1 < remainingCount; value++)
			values[value] = values[value + 1];
		remainingCount -= 1;
	}
}

void PuzzleCrazyTurtle::activateRandomRuleSlots(bool *slots, int count) {
	int available[kRuleSlotCount] = {0, 1, 2, 3, 4};
	int availableCount = kRuleSlotCount;
	for (int selectedCount = 0; selectedCount < count; selectedCount++) {
		const int selected = _vm->_rnd->getRandomNumber(availableCount - 1);
		slots[available[selected]] = true;
		for (int slot = selected; slot + 1 < availableCount; slot++)
			available[slot] = available[slot + 1];
		availableCount -= 1;
	}
}

void PuzzleCrazyTurtle::placeZoombinis() {
	const uint count = MIN(static_cast<uint>(kZoombiniCount), _puzzleZoombinis.size());
	for (uint index = 0; index < count; index++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[index];
		zoombini->setPosition(kZoombiniPos[index]);
		zoombini->setDefaultAnimation(_zoombiniAnimation, 66);
		zoombini->_inputEnabled = true;
		zoombini->_puzzleStatus = 0;
	}
}

void PuzzleCrazyTurtle::onUpdate() {
	// The initial puzzle state remains stable until the player chooses a
	// Zoombini placement or uses the sidebar to leave the current game.
}

void PuzzleCrazyTurtle::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleCrazyTurtle::onRenderContent(ManagedSurface32 *screen) {
	drawBridgeState(screen);
	drawMother(screen);
	drawTurtles(screen);
	drawRuleHints(screen);
}

void PuzzleCrazyTurtle::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleCrazyTurtle::drawBridgeState(ManagedSurface32 *screen) const {
	if (0 < _remainingMistakes) {
		_vm->_gfx->drawRleBlock(screen, _bridgeImage, Common::Point32(10, 220));
	} else if (_collapsedBridgeImage) {
		_vm->_gfx->drawRleBlock(screen, _collapsedBridgeImage, Common::Point32(10, 220));
	}

	if (!_beamImage)
		return;
	Common::Point32 beamPos(11 * _remainingMistakes + 9, 310 - 12 * _remainingMistakes);
	for (int beam = 0; beam < _remainingMistakes; beam++) {
		_vm->_gfx->drawRleBlock(screen, _beamImage, beamPos);
		beamPos.x -= 11;
		beamPos.y += 12;
	}
}

void PuzzleCrazyTurtle::drawRuleHints(ManagedSurface32 *screen) const {
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		if (!_primaryRuleActive[slot])
			continue;
		const RleBlock *primary = _traitImages[_primaryFeature][_ruleValues[0][slot] - 1];
		_vm->_gfx->drawRleBlock(screen, primary, Common::Point32(kPrimaryIconPos[slot].x - 5, kPrimaryIconPos[slot].y));
		if (3 <= _level) {
			const RleBlock *secondary = _traitImages[_secondaryFeature][_ruleValues[1][slot] - 1];
			_vm->_gfx->drawRleBlock(screen, secondary, Common::Point32(kSecondaryIconPos[slot].x - 5, kSecondaryIconPos[slot].y));
		}
	}
}

void PuzzleCrazyTurtle::drawTurtles(ManagedSurface32 *screen) const {
	for (int index = 0; index < kTurtleCount; index++) {
		const TurtlePlacement &placement = kTurtlePlacements[index];
		const RleBlock *image = _turtleFixedImages[placement.type - 1];
		_vm->_gfx->drawRleBlock(screen, image, placement.pos);
	}
}

void PuzzleCrazyTurtle::drawMother(ManagedSurface32 *screen) const {
	_vm->_gfx->drawRleBlock(screen, _motherStartImage, Common::Point32(488, 310));
}

EventHandleResult PuzzleCrazyTurtle::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	// Three-button navigation is handled before page input. Turtle placement remains
	// unchanged here until the path-driven Zoombini interaction is available.
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
