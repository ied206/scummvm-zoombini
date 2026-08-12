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

#include "zoombini2/pages/crazyturtle.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// CrazyTurtlePuzzle — Bridge building puzzle.
//
// Original: CrazyTurtle__Init_416660. Object size 0xBC9C (48284 bytes).
//
// Core mechanics:
//   - 10 turtles arranged around a gap/chasm
//   - Clicking a turtle makes it "spin" (activate)
//   - Activated turtles of matching types form bridge segments
//   - Zoombinis must be matched to correct turtle by traits
//   - When bridge segment is complete, matched zoombini can cross
//   - Goal: Get 4+ zoombinis across
//
// Turtle positions are defined in Area.bmt
// Movement paths: 1.PAT, 2.PAT, 3.PAT, EXIT.PAT
// ============================================================================

// Animation timing (ms)
static const uint32 kTurtleSpinDuration = 2000;
static const uint32 kZoombiniCrossDuration = 2000;
static const uint32 kMotherSpeakDuration = 3000;

// Turtle hitbox size
static const int kTurtleHitSize = 60;

// Example turtle positions (to be refined from Area.bmt)
// Original places 10 turtles around the screen edges
static const int kTurtlePositions[10][2] = {
	{  80, 200 },   // Turtle 0
	{ 160, 150 },   // Turtle 1
	{ 240, 120 },   // Turtle 2
	{ 320, 100 },   // Turtle 3
	{ 400, 120 },   // Turtle 4
	{ 480, 150 },   // Turtle 5
	{ 560, 200 },   // Turtle 6
	{ 200, 400 },   // Turtle 7
	{ 320, 420 },   // Turtle 8
	{ 440, 400 }    // Turtle 9
};

// Mother turtle position
static const int kMotherX = 60;
static const int kMotherY = 350;

CrazyTurtlePuzzle::CrazyTurtlePuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageCrazyTurtle),
	  _state(kStateInit),
	  _freedCount(0),
	  _currentTurtle(-1),
	  _numTurtles(0),
	  _activatedCount(0),
	  _motherState(0),
	  _motherSpeaking(false),
	  _motherAnim(nullptr),
	  _motherDebutGfx(nullptr),
	  _motherFinGfx(nullptr),
	  _motherSpeakAnim(nullptr),
	  _smokeyAnim(nullptr),
	  _zombisTombeGfx(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < kTurtleTypeCount; i++) {
		_turtleIdleAnim[i] = nullptr;
		_turtleSpinAnim[i] = nullptr;
		_turtleFixedGfx[i] = nullptr;
	}

	for (int i = 0; i < kMaxTurtles; i++) {
		_turtles[i].active = false;
		_turtles[i].state = kTurtleInactive;
	}

}

CrazyTurtlePuzzle::~CrazyTurtlePuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	// Free turtle resources
	for (int i = 0; i < kTurtleTypeCount; i++) {
		delete _turtleIdleAnim[i];
		delete _turtleSpinAnim[i];
		delete _turtleFixedGfx[i];
	}

	// Free mother turtle resources
	delete _motherAnim;
	delete _motherDebutGfx;
	delete _motherFinGfx;
	delete _motherSpeakAnim;

	// Free effects
	delete _smokeyAnim;
	delete _zombisTombeGfx;
}

void CrazyTurtlePuzzle::init() {
	// Call base init for background and zoombini loading
	PuzzlePage::init();

	// BGM: 08-BS01.wav (IDA: CrazyTurtle__Init_416660)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/08-BS01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);

	debug(1, "CrazyTurtlePuzzle::init — difficulty %d", diff);

	// Set error tolerance based on difficulty
	switch (diff) {
	case 1: _maxWrongCount = 3; break;
	case 2: _maxWrongCount = 4; break;
	case 3: _maxWrongCount = 6; break;
	default: _maxWrongCount = 3; break;
	}

	// Load resources
	loadResources();

	// Setup turtles based on difficulty
	setupTurtles();

	_freedCount = 0;
	_activatedCount = 0;
	_currentTurtle = -1;
	_motherState = 0;  // debut
	_motherSpeaking = false;
	_state = kStateIdle;
	_stateTimer = _engine->getGameTickCount();
	_wrongCount = 0;
	_currentSequenceIdx = 0;
	
	// Generate target sequence of 16 turtles (random for now)
	Common::RandomSource rnd("crazyturtle_seq");
	_targetSequence.clear();
	for (int i = 0; i < 16; i++) {
		_targetSequence.push_back(rnd.getRandomNumber(_numTurtles - 1));
	}
	
	debug(2, "CrazyTurtlePuzzle: Initialized with maxWrongCount=%d, sequence size=%d", 
		   _maxWrongCount, (int)_targetSequence.size());
}

void CrazyTurtlePuzzle::loadResources() {
	loadTurtleResources();
}

void CrazyTurtlePuzzle::loadTurtleResources() {
	// Load turtle animations for each of the 4 types

	// Idle (attente) animations
	for (int i = 0; i < kTurtleTypeCount; i++) {
		Common::Path idlePath(Common::String::format("bmp/crazy_turtle/tortues/attente/%d/%d", i + 1, i + 1));
		_turtleIdleAnim[i] = new Animation();
		if (!_turtleIdleAnim[i]->loadFromFile(idlePath)) {
			delete _turtleIdleAnim[i];
			_turtleIdleAnim[i] = nullptr;
		}
	}

	// Spinning (tourbillonne) animations
	for (int i = 0; i < kTurtleTypeCount; i++) {
		Common::Path spinPath(Common::String::format("bmp/crazy_turtle/tortues/tourbillonne/%d/%d", i + 1, i + 1));
		_turtleSpinAnim[i] = new Animation();
		if (!_turtleSpinAnim[i]->loadFromFile(spinPath)) {
			delete _turtleSpinAnim[i];
			_turtleSpinAnim[i] = nullptr;
		}
	}

	// Fixed (fixe) sprites when turtle is part of bridge
	for (int i = 0; i < kTurtleTypeCount; i++) {
		Common::Path fixePath(Common::String::format("bmp/crazy_turtle/tortues/tourbillonne/%d/fixe%d", i + 1, i + 1));
		_turtleFixedGfx[i] = new RleBlock();
		if (!_turtleFixedGfx[i]->loadFromFile(fixePath)) {
			delete _turtleFixedGfx[i];
			_turtleFixedGfx[i] = nullptr;
		}
	}

	// Mother turtle
	Common::Path motherPath("bmp/crazy_turtle/tortues/mere/mere");
	_motherAnim = new Animation();
	if (!_motherAnim->loadFromFile(motherPath)) {
		delete _motherAnim;
		_motherAnim = nullptr;
	}

	Common::Path motherDebutPath("bmp/crazy_turtle/tortues/mere/meredebut");
	_motherDebutGfx = new RleBlock();
	if (!_motherDebutGfx->loadFromFile(motherDebutPath)) {
		delete _motherDebutGfx;
		_motherDebutGfx = nullptr;
	}

	Common::Path motherFinPath("bmp/crazy_turtle/tortues/mere/merefin");
	_motherFinGfx = new RleBlock();
	if (!_motherFinGfx->loadFromFile(motherFinPath)) {
		delete _motherFinGfx;
		_motherFinGfx = nullptr;
	}

	Common::Path motherSpeakPath("bmp/crazy_turtle/tortues/mere/parle/parle");
	_motherSpeakAnim = new Animation();
	if (!_motherSpeakAnim->loadFromFile(motherSpeakPath)) {
		delete _motherSpeakAnim;
		_motherSpeakAnim = nullptr;
	}

	// Smoke effect
	Common::Path smokeyPath("bmp/crazy_turtle/smokey");
	_smokeyAnim = new Animation();
	if (!_smokeyAnim->loadFromFile(smokeyPath)) {
		delete _smokeyAnim;
		_smokeyAnim = nullptr;
	}

	debug(2, "CrazyTurtlePuzzle: Turtle resources loaded");
}

void CrazyTurtlePuzzle::setupTurtles() {
	// Setup 10 turtles with random types based on difficulty
	Common::RandomSource rnd("crazyturtle");

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);
	_numTurtles = 10;

	for (int i = 0; i < _numTurtles; i++) {
		_turtles[i].x = kTurtlePositions[i][0];
		_turtles[i].y = kTurtlePositions[i][1];

		// Assign type based on pattern that allows matching
		// Higher difficulty = more varied patterns
		_turtles[i].type = (TurtleType)(rnd.getRandomNumber(kTurtleTypeCount - 1));

		_turtles[i].state = kTurtleIdle;
		_turtles[i].hitbox = Common::Rect(
			_turtles[i].x - kTurtleHitSize / 2,
			_turtles[i].y - kTurtleHitSize / 2,
			_turtles[i].x + kTurtleHitSize / 2,
			_turtles[i].y + kTurtleHitSize / 2
		);
		_turtles[i].animStart = 0;
		_turtles[i].active = true;
	}

	debug(2, "CrazyTurtlePuzzle: Setup %d turtles (diff %d)", _numTurtles, diff);
}

// Removed setupBridge() as it is no longer needed for the correct mechanics.

void CrazyTurtlePuzzle::clickTurtle(int turtleIdx) {
	if (turtleIdx < 0 || turtleIdx >= _numTurtles)
		return;

	Turtle &turtle = _turtles[turtleIdx];
	if (!turtle.active || turtle.state != kTurtleIdle)
		return;

	// Start spinning animation
	turtle.state = kTurtleSpinning;
	turtle.animStart = _engine->getGameTickCount();
	_currentTurtle = turtleIdx;
	_state = kStateTurtleSpinning;
	_stateTimer = _engine->getGameTickCount();

	debug(2, "CrazyTurtlePuzzle: Clicked turtle %d (type %d)", turtleIdx, turtle.type);
}

void CrazyTurtlePuzzle::updateTurtleAnimation(int turtleIdx) {
	if (turtleIdx < 0 || turtleIdx >= _numTurtles)
		return;

	Turtle &turtle = _turtles[turtleIdx];
	if (turtle.state != kTurtleSpinning)
		return;

	uint32 elapsed = _engine->getGameTickCount() - turtle.animStart;
	if (elapsed >= kTurtleSpinDuration) {
		// Spinning done - check if it was the correct turtle in the sequence
		if (_currentSequenceIdx < (int)_targetSequence.size() && 
			turtleIdx == _targetSequence[_currentSequenceIdx]) {
			
			// CORRECT: Zoombini stays on the turtle
			turtle.state = kTurtleFixed;
			_freedCount++;
			_currentSequenceIdx++;
			_state = kStateZoombiniCrossing;
			debug(2, "CrazyTurtlePuzzle: Correct turtle %d placed!", turtleIdx);
		} else {
			// INCORRECT: Zoombini flips into water
			_wrongCount++;
			_state = kStateZoombiniFalling;
			debug(2, "CrazyTurtlePuzzle: Incorrect turtle %d! Wrong count: %d", turtleIdx, _wrongCount);
		}
		_stateTimer = _engine->getGameTickCount();
	}
}

// Removed checkBridgeComplete() as it is no longer needed.
// Removed buildBridgeSegment() as it is no longer needed.

void CrazyTurtlePuzzle::startZoombiniCrossing() {
	// Zoombini crosses safely to the turtle
	_state = kStateZoombiniCrossing;
	_stateTimer = _engine->getGameTickCount();
}

void CrazyTurtlePuzzle::startZoombiniFalling() {
	// Zoombini flips into water
	_state = kStateZoombiniFalling;
	_stateTimer = _engine->getGameTickCount();
}

void CrazyTurtlePuzzle::freeZoombini(int zoombiniIdx) {
	if (zoombiniIdx < 0 || zoombiniIdx >= (int)_puzzleZoombinis.size())
		return;

	_puzzleZoombinis[zoombiniIdx]->_freeStatus = 0;
	_freedCount++;

	debug(1, "CrazyTurtlePuzzle: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int CrazyTurtlePuzzle::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

void CrazyTurtlePuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		break;

	case kStateIdle:
		// Wait for player clicks
		break;

	case kStateTurtleClicked:
		// Brief pause before spinning
		if (elapsed > 100) {
			_state = kStateTurtleSpinning;
			_stateTimer = now;
		}
		break;

	case kStateTurtleSpinning:
		// Update spinning animation
		if (_currentTurtle >= 0) {
			updateTurtleAnimation(_currentTurtle);
		}
		break;

	case kStateZoombiniCrossing:
		// Zoombini crossing animation
		if (elapsed > kZoombiniCrossDuration) {
			// Zoombini successfully placed on turtle
			_state = kStateIdle;
			_stateTimer = now;
			
			if (_freedCount >= 16) {
				_motherState = 2;  // merefin
				_state = kStateDone;
			}
		}
		break;

	case kStateZoombiniFalling:
		// Zoombini falls into water
		if (elapsed > kZoombiniCrossDuration) {
			// Zoombini pops up on the correct turtle
			int correctTurtIdx = _targetSequence[_currentSequenceIdx];
			_turtles[correctTurtIdx].state = kTurtleFixed;
			_freedCount++;
			_currentSequenceIdx++;
			
			// Check if we lost too many support beams
			if (_wrongCount > _maxWrongCount) {
				_state = kStateDone;
				debug(1, "CrazyTurtlePuzzle: Dock collapsed! Wrong count %d > max %d", 
					   _wrongCount, _maxWrongCount);
			} else {
				_state = kStateIdle;
			}
			_stateTimer = now;
		}
		break;

	case kStateMotherSpeaking:
		// Mother turtle speaking
		if (elapsed > kMotherSpeakDuration) {
			_motherSpeaking = false;
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		// Wait before transitioning
		if (elapsed > 2000) {
			debug(1, "CrazyTurtlePuzzle: Complete, %d zoombinis freed", _freedCount);
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageCrazyTurtle;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void CrazyTurtlePuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	// Draw turtles
	drawTurtles(screen);

	// Draw mother turtle
	drawMother(screen);

	// Draw zoombinis
	drawZoombinis(screen);
}

void CrazyTurtlePuzzle::drawTurtles(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numTurtles; i++) {
		const Turtle &turtle = _turtles[i];
		if (!turtle.active)
			continue;

		int drawX = turtle.x - 30;  // Center sprite
		int drawY = turtle.y - 30;
		int type = turtle.type;

		switch (turtle.state) {
		case kTurtleIdle:
			// Draw idle animation frame
			if (_turtleIdleAnim[type]) {
				uint32 elapsed = _engine->getGameTickCount() - turtle.animStart;
				int frame = (elapsed / 100) % _turtleIdleAnim[type]->getFrameCount();
				const RleBlock *frameGfx = _turtleIdleAnim[type]->getFrame(frame);
				if (frameGfx)
					frameGfx->drawToScreen(screen, drawX, drawY, lut);
			}
			break;

		case kTurtleSpinning:
			// Draw spinning animation
			if (_turtleSpinAnim[type]) {
				uint32 elapsed = _engine->getGameTickCount() - turtle.animStart;
				int frame = (elapsed / 50) % _turtleSpinAnim[type]->getFrameCount();
				const RleBlock *frameGfx = _turtleSpinAnim[type]->getFrame(frame);
				if (frameGfx)
					frameGfx->drawToScreen(screen, drawX, drawY, lut);
			}
			break;

		case kTurtleFixed:
			// Draw fixed sprite
			if (_turtleFixedGfx[type]) {
				_turtleFixedGfx[type]->drawToScreen(screen, drawX, drawY, lut);
			}
			break;

		default:
			break;
		}
	}
}

void CrazyTurtlePuzzle::drawMother(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (_motherSpeaking && _motherSpeakAnim) {
		uint32 elapsed = _engine->getGameTickCount() - _stateTimer;
		int frame = (elapsed / 100) % _motherSpeakAnim->getFrameCount();
		const RleBlock *frameGfx = _motherSpeakAnim->getFrame(frame);
		if (frameGfx)
			frameGfx->drawToScreen(screen, kMotherX, kMotherY, lut);
	} else {
		// Draw static mother based on state
		RleBlock *gfx = nullptr;
		switch (_motherState) {
		case 0:  // debut
			gfx = _motherDebutGfx;
			break;
		case 2:  // fin
			gfx = _motherFinGfx;
			break;
		default:
			// Normal state - use animation frame 0
			if (_motherAnim) {
				const RleBlock *frameGfx = _motherAnim->getFrame(0);
				if (frameGfx)
					frameGfx->drawToScreen(screen, kMotherX, kMotherY, lut);
			}
			return;
		}

		if (gfx)
			gfx->drawToScreen(screen, kMotherX, kMotherY, lut);
	}
}

void CrazyTurtlePuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (!_zoombiniGfx)
		return;

	// Draw zoombinis waiting
	int waitX = 50;
	int waitY = 450;
	int idx = 0;

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus != 0) {
			// Not yet freed - draw waiting
			const Zoombini *z = _puzzleZoombinis[i];
			int x = waitX + (idx % 4) * 25;
			int y = waitY - (idx / 4) * 30;

			// Draw zoombini
			int baseIdx = 0;
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);

			const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
			for (int feat = 1; feat <= 4; feat++) {
				int featIdx = baseIdx + feat * ZoombiniGfx::kDim2 + features[feat - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, x, y, lut);
			}

			idx++;
		}
	}
}

void CrazyTurtlePuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

	// Check if clicked on a turtle
	for (int i = 0; i < _numTurtles; i++) {
		if (_turtles[i].active && _turtles[i].state == kTurtleIdle) {
			if (_turtles[i].hitbox.contains(pos)) {
				debug(2, "CrazyTurtlePuzzle: Click at %d,%d hit turtle %d", pos.x, pos.y, i);
				clickTurtle(i);
				return;
			}
		}
	}

	debug(3, "CrazyTurtlePuzzle: Click at %d,%d (no turtle)", pos.x, pos.y);
}

} // End of namespace Zoombini2
