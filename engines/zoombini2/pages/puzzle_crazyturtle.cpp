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

#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/pages/puzzle_crazyturtle.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const CrazyTurtlePuzzle::TurtlePlacement CrazyTurtlePuzzle::kTurtlePlacements[kTurtleCount] = {
	{3,   0, 322}, {3,   8, 383}, {3,  29, 448}, {4, 104, 482},
	{4, 178, 463}, {2, 264, 444}, {1, 245, 378}, {3, 233, 308},
	{4, 248, 238}, {4, 325, 196}, {2, 412, 175}, {1, 492, 165},
	{3, 576, 172}, {2, 548, 245}, {2, 461, 301}, {3, 529, 346}
};

const Common::Point CrazyTurtlePuzzle::kZoombiniPositions[kZoombiniCount] = {
	Common::Point(247, 120), Common::Point(245,  74), Common::Point(202, 117), Common::Point(200,  64),
	Common::Point(166,  42), Common::Point(166,  87), Common::Point(146, 131), Common::Point( 99, 120),
	Common::Point(131,  75), Common::Point(128,  30), Common::Point( 86,  66), Common::Point( 62, 105),
	Common::Point( 49,  48), Common::Point( 18, 135), Common::Point( 25,  91), Common::Point(  8,  55)
};

const Common::Point CrazyTurtlePuzzle::kPrimaryIconPositions[kRuleSlotCount] = {
	Common::Point(365, 15), Common::Point(395, 22), Common::Point(423, 28), Common::Point(450, 35), Common::Point(477, 42)
};

const Common::Point CrazyTurtlePuzzle::kSecondaryIconPositions[kRuleSlotCount] = {
	Common::Point(365, 40), Common::Point(395, 47), Common::Point(423, 54), Common::Point(450, 60), Common::Point(477, 67)
};

CrazyTurtlePuzzle::CrazyTurtlePuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageCrazyTurtle), _difficulty(1), _primaryFeature(0), _secondaryFeature(1), _remainingMistakes(0), _initialMistakes(0),
	  _motherAnimation(nullptr), _motherSpeechAnimation(nullptr), _smokeAnimation(nullptr), _motherStartGraphic(nullptr), _motherEndGraphic(nullptr),
	  _bridgeGraphic(nullptr), _collapsedBridgeGraphic(nullptr), _beamGraphic(nullptr), _musicId(-1) {
	memset(_ruleValues, 0, sizeof(_ruleValues));
	memset(_primaryRuleActive, 0, sizeof(_primaryRuleActive));
	memset(_secondaryRuleActive, 0, sizeof(_secondaryRuleActive));
	for (int type = 0; type < kFeatureCount; type++) {
		_turtleIdleAnimations[type] = nullptr;
		_turtleSpinAnimations[type] = nullptr;
		_turtleFixedGraphics[type] = nullptr;
		for (int value = 0; value < kFeatureValueCount; value++)
			_traitGraphics[type][value] = nullptr;
	}
}

CrazyTurtlePuzzle::~CrazyTurtlePuzzle() {
	if (0 <= _musicId) {
		SoundManager *soundManager = _engine->getSoundManager();
		soundManager->stop(_musicId);
		soundManager->unload(_musicId);
	}
	for (int type = 0; type < kFeatureCount; type++) {
		delete _turtleIdleAnimations[type];
		delete _turtleSpinAnimations[type];
		delete _turtleFixedGraphics[type];
		for (int value = 0; value < kFeatureValueCount; value++)
			delete _traitGraphics[type][value];
	}
	delete _motherAnimation;
	delete _motherSpeechAnimation;
	delete _smokeAnimation;
	delete _motherStartGraphic;
	delete _motherEndGraphic;
	delete _bridgeGraphic;
	delete _collapsedBridgeGraphic;
	delete _beamGraphic;
}

bool CrazyTurtlePuzzle::loadAnimationResource(Animation *&resource, const Common::Path &path) {
	resource = new Animation();
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load animation '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

bool CrazyTurtlePuzzle::loadRleResource(RleBlock *&resource, const Common::Path &path) {
	resource = new RleBlock();
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load RLE graphic '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

void CrazyTurtlePuzzle::init() {
	PuzzlePage::init();

	GameState *gameState = _engine->getGameState();
	_difficulty = CLIP(gameState ? gameState->_gameMode : 1, 1, 4);
	generateRules();
	loadResources();
	placeZoombinis();

	if (SoundManager *soundManager = _engine->getSoundManager()) {
		_musicId = soundManager->load(true, Common::Path("sounds/music/08-BS01.wav"), true);
		if (0 <= _musicId) {
			soundManager->playLoop(_musicId);
			soundManager->setVolume(_musicId, soundManager->_volumeMusic);
		}
	}

	if (gameState && _engine->_isSavedGame)
		gameState->registerWorldVisit(kPageCrazyTurtle, 1);

	debug(1, "CrazyTurtlePuzzle::init: difficulty=%d primary=%d secondary=%d mistakes=%d party=%u", _difficulty, _primaryFeature,
	      _secondaryFeature, _remainingMistakes, _puzzleZoombinis.size());
}

void CrazyTurtlePuzzle::loadResources() {
	for (int type = 0; type < kFeatureCount; type++) {
		const int resourceNumber = type + 1;
		loadAnimationResource(_turtleIdleAnimations[type],
		                      Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/ATTENTE/%d/%d.AN", resourceNumber, resourceNumber)));
		loadAnimationResource(_turtleSpinAnimations[type],
		                      Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/tourbillonne/%d/%d.AN", resourceNumber, resourceNumber)));
		loadRleResource(_turtleFixedGraphics[type],
		                Common::Path(Common::String::format("Bmp/crazy_turtle/TORTUES/tourbillonne/%d/FIXE%d.RB", resourceNumber, resourceNumber)));
	}

	loadAnimationResource(_motherAnimation, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/mere.an"));
	loadAnimationResource(_motherSpeechAnimation, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/PARLE/PARLE.AN"));
	loadAnimationResource(_smokeAnimation, Common::Path("Bmp/crazy_turtle/smokey.an"));
	loadRleResource(_motherStartGraphic, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/meredebut.rb"));
	loadRleResource(_motherEndGraphic, Common::Path("Bmp/crazy_turtle/TORTUES/MERE/MEREFIN.RB"));
	loadRleResource(_bridgeGraphic, Common::Path("Bmp/crazy_turtle/PONT.RB"));
	loadRleResource(_collapsedBridgeGraphic, Common::Path("Bmp/crazy_turtle/pontKC.rb"));
	loadRleResource(_beamGraphic, Common::Path("Bmp/crazy_turtle/poutrelle.rb"));

	for (int feature = 0; feature < kFeatureCount; feature++) {
		for (int value = 0; value < kFeatureValueCount; value++) {
			loadRleResource(_traitGraphics[feature][value],
			                Common::Path(Common::String::format("Bmp/mystic_marsh/TRAITS/%d-%d.RB", feature + 1, value + 1)));
		}
	}
}

void CrazyTurtlePuzzle::generateRules() {
	Common::RandomSource *randomSource = _engine->getRandom();
	_primaryFeature = randomSource->getRandomNumber(kFeatureCount - 1);
	int remainingFeatures[kFeatureCount - 1];
	int remainingCount = 0;
	for (int feature = 0; feature < kFeatureCount; feature++) {
		if (feature != _primaryFeature) {
			remainingFeatures[remainingCount] = feature;
			remainingCount += 1;
		}
	}
	_secondaryFeature = remainingFeatures[randomSource->getRandomNumber(remainingCount - 1)];

	for (int group = 0; group < kRuleGroupCount; group++)
		generateFeatureOrder(group);
	memset(_primaryRuleActive, 0, sizeof(_primaryRuleActive));
	memset(_secondaryRuleActive, 0, sizeof(_secondaryRuleActive));

	switch (_difficulty) {
	case 1:
		_remainingMistakes = 3;
		for (int slot = 0; slot < kRuleSlotCount; slot++)
			_primaryRuleActive[slot] = true;
		break;
	case 2: {
		const int visibleRuleCount = randomSource->getRandomNumber(1) + 1;
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

void CrazyTurtlePuzzle::generateFeatureOrder(int group) {
	int values[kFeatureValueCount] = {1, 2, 3, 4, 5};
	int remainingCount = kFeatureValueCount;
	Common::RandomSource *randomSource = _engine->getRandom();
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		const int selected = randomSource->getRandomNumber(remainingCount - 1);
		_ruleValues[group][slot] = values[selected];
		for (int value = selected; value + 1 < remainingCount; value++)
			values[value] = values[value + 1];
		remainingCount -= 1;
	}
}

void CrazyTurtlePuzzle::activateRandomRuleSlots(bool *slots, int count) {
	int available[kRuleSlotCount] = {0, 1, 2, 3, 4};
	int availableCount = kRuleSlotCount;
	Common::RandomSource *randomSource = _engine->getRandom();
	for (int selectedCount = 0; selectedCount < count; selectedCount++) {
		const int selected = randomSource->getRandomNumber(availableCount - 1);
		slots[available[selected]] = true;
		for (int slot = selected; slot + 1 < availableCount; slot++)
			available[slot] = available[slot + 1];
		availableCount -= 1;
	}
}

void CrazyTurtlePuzzle::placeZoombinis() {
	const uint count = MIN(static_cast<uint>(kZoombiniCount), _puzzleZoombinis.size());
	for (uint index = 0; index < count; index++) {
		Zoombini *zoombini = _puzzleZoombinis[index];
		zoombini->_posX = kZoombiniPositions[index].x;
		zoombini->_posY = kZoombiniPositions[index].y;
		zoombini->_targetX = zoombini->_posX;
		zoombini->_targetY = zoombini->_posY;
		zoombini->_activeFlag = 1;
		zoombini->_freeStatus = 0;
	}
}

void CrazyTurtlePuzzle::update() {
	// The initial puzzle state remains stable until the player chooses a
	// Zoombini placement or uses the sidebar to leave the current game.
}

void CrazyTurtlePuzzle::draw(Graphics::ManagedSurface *screen) {
	if (_background)
		_background->drawToSurface(screen, 0, 0);
	drawBridgeState(screen);
	drawMother(screen);
	drawTurtles(screen);
	drawRuleHints(screen);
	drawZoombinis(screen);
}

void CrazyTurtlePuzzle::drawBridgeState(Graphics::ManagedSurface *screen) const {
	const byte (*alphaLUT)[256] = _engine->getAlphaLUT();
	if (0 < _remainingMistakes) {
		if (_bridgeGraphic)
			_bridgeGraphic->drawToScreen(screen, 10, 220, alphaLUT);
	} else if (_collapsedBridgeGraphic) {
		_collapsedBridgeGraphic->drawToScreen(screen, 10, 220, alphaLUT);
	}

	if (!_beamGraphic)
		return;
	int x = 11 * _remainingMistakes + 9;
	int y = 310 - 12 * _remainingMistakes;
	for (int beam = 0; beam < _remainingMistakes; beam++) {
		_beamGraphic->drawToScreen(screen, x, y, alphaLUT);
		x -= 11;
		y += 12;
	}
}

void CrazyTurtlePuzzle::drawRuleHints(Graphics::ManagedSurface *screen) const {
	const byte (*alphaLUT)[256] = _engine->getAlphaLUT();
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		if (!_primaryRuleActive[slot])
			continue;
		const RleBlock *primary = _traitGraphics[_primaryFeature][_ruleValues[0][slot] - 1];
		if (primary)
			primary->drawToScreen(screen, kPrimaryIconPositions[slot].x - 5, kPrimaryIconPositions[slot].y, alphaLUT);
		if (3 <= _difficulty) {
			const RleBlock *secondary = _traitGraphics[_secondaryFeature][_ruleValues[1][slot] - 1];
			if (secondary)
				secondary->drawToScreen(screen, kSecondaryIconPositions[slot].x - 5, kSecondaryIconPositions[slot].y, alphaLUT);
		}
	}
}

void CrazyTurtlePuzzle::drawTurtles(Graphics::ManagedSurface *screen) const {
	const byte (*alphaLUT)[256] = _engine->getAlphaLUT();
	for (int index = 0; index < kTurtleCount; index++) {
		const TurtlePlacement &placement = kTurtlePlacements[index];
		const RleBlock *graphic = _turtleFixedGraphics[placement.type - 1];
		if (graphic)
			graphic->drawToScreen(screen, placement.x, placement.y, alphaLUT);
	}
}

void CrazyTurtlePuzzle::drawMother(Graphics::ManagedSurface *screen) const {
	if (_motherStartGraphic)
		_motherStartGraphic->drawToScreen(screen, 488, 310, _engine->getAlphaLUT());
}

void CrazyTurtlePuzzle::drawZoombinis(Graphics::ManagedSurface *screen) const {
	const uint count = MIN(static_cast<uint>(kZoombiniCount), _puzzleZoombinis.size());
	for (uint index = 0; index < count; index++)
		drawZoombini(screen, *_puzzleZoombinis[index]);
}

void CrazyTurtlePuzzle::drawZoombini(Graphics::ManagedSurface *screen, const Zoombini &zoombini) const {
	if (!_zoombiniGfx)
		return;
	const byte (*alphaLUT)[256] = _engine->getAlphaLUT();
	static constexpr int kWaitingCell = 66;
	const int baseIndex = kWaitingCell * ZoombiniGfx::kDim1 * ZoombiniGfx::kDim2;
	const RleBlock *frame = _zoombiniGfx->getFrame(baseIndex, 0);
	if (frame)
		frame->drawToScreen(screen, zoombini._posX, zoombini._posY, alphaLUT);
	const byte features[kFeatureCount] = {zoombini._featureA, zoombini._featureB, zoombini._featureC, zoombini._featureD};
	for (int layer = 1; layer <= kFeatureCount; layer++) {
		frame = _zoombiniGfx->getFrame(baseIndex + layer * ZoombiniGfx::kDim2 + features[layer - 1], 0);
		if (frame)
			frame->drawToScreen(screen, zoombini._posX, zoombini._posY, alphaLUT);
	}
}

void CrazyTurtlePuzzle::handleClick(const Common::Point &pos) {
	(void)pos;
	// Sidebar navigation is handled before page input. Turtle placement remains
	// unchanged here until the path-driven Zoombini interaction is available.
}

} // End of namespace Zoombini2
